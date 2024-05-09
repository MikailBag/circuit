#pragma once

#include "conf/conf.h"

#include <memory>

namespace bf {
struct EvalAlphaConfig: public conf::Target {
    void Describe(conf::Description& desc) override {
        desc.Object();
    }

    void Postprocess() override {
    }
};
struct EvalBetaConfig : public conf::Target {
    bool skipValidation = false;
    bool forceDummy = false;

    void Describe(conf::Description& desc) override {
        desc.Object()
            .BoolField("skip_validation", skipValidation)
            .BoolField("force_dummy", forceDummy);
    }

    void Postprocess() override {
        if (skipValidation) {
            throw conf::BindingException("TODO");
        }
    }
};
struct EvalConfig : public conf::Target {
    EvalAlphaConfig alpha;
    bool isAlpha = false;
    EvalBetaConfig beta;
    bool isBeta = false;

    void Describe(conf::Description& desc) override {
        desc.Enum()
            .Variant("alpha", isAlpha, alpha)
            .Variant("beta", isBeta, beta);
    }

    void Postprocess() override {
        if (!isAlpha && !isBeta) {
            isBeta = true;
        }
    }
};
struct FilterConfig : public conf::Target {
    bool unsafe = false;

    void Describe(conf::Description& desc) override {
        desc.Object()
            .BoolField("unsafe", unsafe);
    }

    void Postprocess() override {
    }
};

struct LaunchConfig : public conf::Target {
    bool parallel = false;
    size_t threadCount = 8;
    size_t chunkSize = 10;

    void Describe(conf::Description& desc) override {
        desc.Object()
            .BoolField("parallel", parallel)
            .NumField("thread_count", threadCount)
            .NumField("chunk_size", chunkSize);
    }

    void Postprocess() override;
};
}
