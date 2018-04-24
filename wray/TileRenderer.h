#pragma once

class TileTask
{
public:
	enum TaskStatus
	{
		WAITING,
		EXECUTING,
		FINISHED,
	};
	TileTask(int beginWidth, int beginHeight, int w, int h) :
		m_beginWidth(beginWidth), m_beginHeight(beginHeight),
		m_width(w), m_height(h), m_status(WAITING), m_currentPass(0)
	{
		m_result.resize(m_width*m_height);
	}

	void resetTask() { m_status = WAITING; }
	void finishTask() { m_status = FINISHED; m_currentPass++; }
	int m_beginWidth, m_beginHeight;
	int m_width, m_height;
	int m_currentPass;
	TaskStatus m_status;
private:
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
	enum RenderStatus
	{
		NOT_RENDERED,
		WAITING_TO_RENDER,
		RENDERING,
		WAITING_TO_STOP,
	};
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
	Camera* getCamera() { return m_camera.get(); }

	// Render
	void beginRender();
	void stopRender();
	bool isRendering();

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
	RenderStatus m_renderStatus;

	// Scene Data
	std::unique_ptr<Scene> m_scene;
	std::unique_ptr<WAccelerator> m_tree;

	// Camera
	std::unique_ptr<Camera> m_camera;

	// Renderer itself
	static std::unique_ptr<TileRenderer> s_instance;
};

