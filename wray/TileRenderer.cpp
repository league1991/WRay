#include "stdafx.h"
#include "TileRenderer.h"


std::unique_ptr<TileRenderer> TileRenderer::s_instance;

TileRenderer::TileRenderer() :
	m_width(0), m_height(0), m_tileWidth(32), m_tileHeight(32),
	m_tileCountWidth(0), m_tileCountHeight(0), m_isRendering(0), m_renderPass(0),
	m_camera(new Camera) {
	for (int i = 0; i < getSystemCores(); i++)
	{
		m_threads.emplace_back(new RenderThread());
	}

	m_scheduleThread = std::unique_ptr<std::thread>(new std::thread([this]() {
		while (true) {
			scheduleTasks();
			std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1000));
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
	WObjReader reader;
	reader.readFile("areaLight.obj");
	m_scene.reset(new Scene());
	m_scene->buildScene(reader);

	m_tree = std::unique_ptr<WMultiBVH>(new WMultiBVH());
	m_tree->buildTree(*m_scene);
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
	if (m_isRendering == false)
	{
		return;
	}
	// Collect free threads
	std::vector<RenderThread*> freeThreads;
	for (auto& threadPtr : m_threads) {
		if (!threadPtr->isWorking()) {
			freeThreads.push_back(threadPtr.get());
		}
	}

	// Handle tasks.
	int nUnfinishedTasks = 0;
	for (auto task : m_tileTask)
	{
		if (!task->isFinished())
		{
			nUnfinishedTasks++;
		}
		else if (freeThreads.size() != 0)
		{
			auto lastThread = freeThreads.back();
			lastThread->runTask(task);
			freeThreads.erase(freeThreads.end() - 1);
		}
	}

	// Finish one pass
	if (nUnfinishedTasks == 0)
	{
		for (auto task : m_tileTask)
		{
			task->initTask();
		}
		m_renderPass++;
	}
}

bool TileRenderer::resize(int width, int height)
{
	if (m_isRendering)
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
	m_camera->setParameter(origin, target, up, fov, ratioWH * 0.5);
	m_camera->changeSampleSize(1);
	resize(width, height);
}

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
						WRay r;
						camera->getNextRay(r, x, y);
						color += m_integrator->integrate(r);
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
}
