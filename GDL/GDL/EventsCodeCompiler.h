#if defined(GD_IDE_ONLY)

#ifndef EVENTSCODECOMPILER_H
#define EVENTSCODECOMPILER_H
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>
#include <SFML/System.hpp>
class EventsExecutionEngine;
class Scene;
class Game;

/**
 * \brief Launch and manage events compilation tasks.
 */
class GD_API EventsCodeCompiler
{
public:
    void EventsCompilationNeeded(Game & game, Scene & scene);

    void NotifyASceneIsDestroyed(const Scene & scene);

    /**
     * Tell the compiler the events of the scene can be compiled:
     * ( Compiler can launch at this moment a pending task )
     */
    void EnableCompilation(Scene & scene);

    /**
     * Tell the compiler to not attempt to compile anything.
     * However, if a compilation is currently run, it will not stop:
     * That is why the scene has to wait until SceneEventsBeingCompiled return false.
     */
    void DisableCompilation(Scene & scene);

    /**
     * Return true if thread is compiling a scene's events.
     * -> In this case, scenes must not try to launch runtime scenes.
     */
    bool EventsBeingCompiled();

    /**
     * Return true if thread is ( or is going to ) compiling scene's events.
     */
    bool SceneEventsBeingCompiled(const Scene & scene);

    /**
     * A directory where headers can be found
     */
    void AddHeaderDirectory(std::string dir) { headersDirectories.insert(dir); }

    static EventsCodeCompiler * GetInstance()
    {
        if ( NULL == _singleton )
            _singleton = new EventsCodeCompiler;

        return ( static_cast<EventsCodeCompiler*>( _singleton ) );
    }

    static void DestroySingleton()
    {
        if ( NULL != _singleton )
        {
            delete _singleton;
            _singleton = NULL;
        }
    }

    static sf::Mutex openSaveDialogMutex; ///< wxWidgets Open/Save dialog seems to cause crash when writing bitcode at the same time.

private:

    class Worker
    {
    public:
        Worker(Game * game_, Scene * scene_) :
            abort(false),
            workEnded(false),
            game(game_),
            scene(scene_),
            thread(&EventsCodeCompiler::Worker::DoCompleteCompilation, this)
        {
        }
        virtual ~Worker() {};

        void Launch() { thread.Launch(); }
        const Scene* GetScene() { return scene; }

        bool abort;
        bool workEnded;

    private:
        Game * game;
        Scene * scene;
        sf::Thread thread;
        void DoCompleteCompilation();
    };

    /**
     * Compile C++ events file to bitcode file.
     */
    static bool CompileEventsCppFileToBitCode(std::string eventsFile, std::string bitCodeFile);

    void AddPendingTask(Game & game, Scene & scene);

    boost::shared_ptr<Worker> currentTask; ///< Compilation currently made.
    std::vector <std::pair<Scene*, Game*> > pendingTasks; ///< Compilation task waiting to be launched.
    std::vector < Scene* > compilationDisallowed; ///< List of scenes which disallow their events to be compiled. (However, if a compilation is being made, it will not be stopped)

    static std::set<std::string> headersDirectories;
    static sf::Mutex mutex;

    friend class Worker;
    EventsCodeCompiler();
    virtual ~EventsCodeCompiler();
    static EventsCodeCompiler *_singleton;
};

#endif // EVENTSCODECOMPILER_H
#endif
