#include "stdafx.h"
#include "TileRenderer.h"


std::unique_ptr<TileRenderer> TileRenderer::s_instance;

TileRenderer::TileRenderer() :
	m_width(0), m_height(0), m_tileWidth(128), m_tileHeight(128),
	m_tileCountWidth(0), m_tileCountHeight(0), m_renderStatus(NOT_RENDERED), m_renderPass(0),
	m_camera(new Camera) {
	for (int i = 0; i < getSystemCores(); i++)
	{
		m_threads.emplace_back(new RenderThread());
	}

	m_scheduleThread = std::unique_ptr<std::thread>(new std::thread([this]() {
		while (true) {
			scheduleTasks();
			std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100));
		}
	}));
}

TileRenderer::~TileRenderer()
{
}

void TileRenderer::init()
{

}

void TileRenderer::readScene(const std::string & objPath)
{
	ObjReader reader;
	reader.readFile(objPath);
	m_scene.reset(new Scene());
	m_scene->buildScene(reader);

	m_tree = std::unique_ptr<WMultiBVH>(new WMultiBVH());
	m_tree->buildTree(*m_scene);

	for (int i = 0; i < getSystemCores(); i++)
	{
		m_threads[i]->init();
	}
}

TileRenderer * TileRenderer::getInstance()
{
	if (s_instance == nullptr)
	{
		s_instance = std::unique_ptr<TileRenderer>(new TileRenderer());
	}
	return s_instance.get();
}

void TileRenderer::scheduleTasks()
{
	if (m_renderStatus == NOT_RENDERED)
	{
		return;
	}
	if (m_renderStatus == WAITING_TO_RENDER)
	{
		m_camera->clearFilm();
		for (auto task : m_tileTask)
		{
			task->resetTask();
		}
		m_renderStatus = RENDERING;
	}

	// Collect free threads
	int nWorkingThreads = 0;
	std::vector<RenderThread*> freeThreads;
	for (auto& threadPtr : m_threads) {
		if (!threadPtr->isWorking()) {
			freeThreads.push_back(threadPtr.get());
		}
		else
		{
			nWorkingThreads++;
		}
	}
	if (m_renderStatus == WAITING_TO_STOP)
	{
		if (nWorkingThreads == 0)
		{
			m_renderStatus = NOT_RENDERED;
		}
		return;
	}

	// Handle tasks.
	int nUnfinishedTasks = 0;
	for (auto task : m_tileTask)
	{
		if (task->m_status != TileTask::FINISHED)
		{
			nUnfinishedTasks++;
		}
		if (task->m_status == TileTask::WAITING && freeThreads.size() != 0)
		{
			auto lastThread = freeThreads.back();
			lastThread->runTask(task);
			task->m_status = TileTask::EXECUTING;
			freeThreads.erase(freeThreads.end() - 1);
		}
	}

	// Finish one pass
	if (nUnfinishedTasks == 0)
	{
		for (auto task : m_tileTask)
		{
			task->resetTask();
		}
		m_renderPass++;
	}
}

bool TileRenderer::resize(int width, int height)
{
	if (m_renderStatus != NOT_RENDERED)
	{
		return false;
	}
	if (width == m_width && height == m_height)
	{
		return true;
	}

	m_width = width;
	m_height = height;

	m_tileCountWidth = std::ceil((float)m_width / (float)m_tileWidth);
	m_tileCountHeight = std::ceil((float)m_height / (float)m_tileHeight);

	m_camera->setFilmResolutionXY(m_width, m_height);

	m_tileTask.clear();
	m_tileTask.reserve(m_tileCountWidth*m_tileCountHeight);
	for (int i = 0; i < m_tileCountWidth; i++)
	{
		for (int j = 0; j < m_tileCountHeight; j++)
		{
			int beginWidth = i*m_tileWidth;
			int beginHeight = j*m_tileHeight;
			int width = min(m_width - beginWidth, m_tileWidth);
			int height = min(m_height - beginHeight, m_tileHeight);
			m_tileTask.emplace_back(new TileTask(beginWidth, beginHeight, width, height));
		}
	}
	return true;
}

void TileRenderer::setCamera(const Vector3 & origin, const Vector3 & target, const Vector3 & up, float fov, int width, int height) {
	float ratioWH = (float)width / (float)height;
	m_camera->setParameter(origin, target, up, fov, ratioWH);
	m_camera->setFilmResolutionX(width);
	m_camera->changeSampleSize(1);
	resize(width, height);
}

// Render

void TileRenderer::beginRender()
{
	if (m_renderStatus == NOT_RENDERED)
	{
		m_renderStatus = WAITING_TO_RENDER;
	}
}

void TileRenderer::stopRender()
{
	if (m_renderStatus == RENDERING)
	{
		m_renderStatus = WAITING_TO_STOP;
	}
}

bool TileRenderer::isRendering() { return m_renderStatus == RENDERING; }

void RenderThread::run() {
	m_thread = std::unique_ptr<std::thread>(new std::thread([this]() {
		while (true) {
			std::unique_lock<std::mutex> lck(m_mutex);
			if (m_task) {
				auto renderer = TileRenderer::getInstance();
				auto camera = renderer->getCamera();
				for (int y = m_task->m_beginHeight; y < m_task->m_beginHeight + m_task->m_height; ++y)
				{
					for (int x = m_task->m_beginWidth; x < m_task->m_beginWidth+m_task->m_width; ++x)
					{
						Vector3 color(0, 0, 0);
						Ray r;
						camera->getNextRay(r, x, y);
						for (int samples = 0; samples < 4; samples++)
						{
							color += m_integrator->integrate(r);
						}
						color /= 10;
						camera->accumulateColor(color.x, color.y, color.z, x, y);
					}
				}
				m_task->finishTask();
			}
			m_task = nullptr;
			m_cv.wait(lck);
		}
	}));
}

bool RenderThread::runTask(std::shared_ptr<TileTask> newTask)
{
	if (isWorking() || newTask == nullptr) {
		return false;
	}
	m_task = newTask;
	m_cv.notify_all();
	return true;
}

bool RenderThread::init()
{
	if (isWorking())
	{
		return false;
	}
	auto renderer = TileRenderer::getInstance();
	m_integrator.reset(new WPathIntegrator(renderer->getScene(), renderer->getTree(), 20, WSampler::SAMPLER_STRATIFIED, 1.0f));
	return true;
}
