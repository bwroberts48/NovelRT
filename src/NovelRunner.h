//
// Created by matth on 16/02/2019.
//

#ifndef NOVELRT_NOVELRUNNER_H
#define NOVELRT_NOVELRUNNER_H


#include "NovelRenderingService.h"

typedef void (*NovelSubscriber)(const float);

namespace NovelRT {
    class NovelRunner {
    public:
        void runOnUpdate(NovelSubscriber);
        void stopRunningOnUpdate(NovelSubscriber);
        void executeUpdateSubscriptions(const float deltaTime) const;

        explicit NovelRunner(int);
        int runNovel() const;
        std::shared_ptr<NovelRenderingService> getRenderer();

    private:
        std::vector<NovelSubscriber> _updateSubscribers;
        std::shared_ptr<NovelRenderingService> _novelRenderer;
    };
}

#endif //NOVELRT_NOVELRUNNER_H