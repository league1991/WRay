#pragma once

class TileTask
{
public:
	TileTask(int beginWidth, int beginHeight, int w, int h) :
		m_beginWidth(beginWidth), m_beginHeight(beginHeight),
		m_width(w), m_height(h), m_finished(false)
	{
		m_result.resize(m_width*m_height);
	}

	void initTask() { m_finished = false; }
	void finishTask() { m_finished = true; }
	bool isFinished() { return m_finished; }

	int m_beginWidth, m_beginHeight;
	int m_width, m_height;
private:
	bool m_finished;
	std::vector<Vector4> m_result;
};

class RenderThread
{
public:
	RenderThread() { run(); }
	bool isWorking() { return m_task != nullptr; }
	bool runTask(std::shared_ptr<TileTask> newTask);

	bool init();
private:
	void run();

	// Thread sync
	std::mutex                   m_mutex;
	std::condition_variable      m_cv;
	std::shared_ptr<TileTask>    m_task;
	std::unique_ptr<std::thread> m_thread;

	// Per-thread data
	std::unique_ptr<WSurfaceIntegrator> m_integrator;
};

class TileRenderer
{
public:
	TileRenderer();
	~TileRenderer();

	// Initialization
	void init();

	// Scene
	void readScene(const std::string& objPath);
	Scene* getScene() { return m_scene.get(); }
	WAccelerator* getTree() { return m_tree.get(); }

	// Camera
	bool resize(int width, int height);
	void setCamera(const Vector3& origin, const Vector3& target, const Vector3& up, float fov, int width, int height);
	Camera* getCamera() { m_camera; }

	// Render
	void beginRender()
	{
		m_isRendering = true;
	}
	void stopRender()
	{
		m_isRendering = false;
	}

	static TileRenderer* getInstance();
protected:
	void scheduleTasks();

	int getSystemCores() {
		return max(1u, std::thread::hardware_concurrency());
	}

	int renderThreadFunc() {}

	int m_width;
	int m_height;
	int m_tileWidth;
	int m_tileHeight;
	int m_tileCountWidth;
	int m_tileCountHeight;

	int m_renderPass;

	// Thread
	std::vector<std::unique_ptr<RenderThread>> m_threads;
	std::unique_ptr<std::thread> m_scheduleThread;
	std::vector<std::shared_ptr<TileTask>> m_tileTask;
	bool m_isRendering;

	// Scene Data
	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<WAccelerator> m_tree;

	// Camera
	std::unique_ptr<Camera> m_camera;

	// Renderer itself
	static std::unique_ptr<TileRenderer> s_instance;
};

