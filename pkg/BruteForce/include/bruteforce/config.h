#pragma once

#include "conf/conf.h"

#include <memory>

namespace bf {
struct EvalAlphaConfig: public conf::Target {
    void Describe(conf::Description& desc) override {
        desc.IsObject();
    }

    void Postprocess() override {
    }
};
struct EvalBetaConfig : public conf::Target {
    bool skipValidation = false;
    bool forceDummy = false;

    void Describe(conf::Description& desc) override {
        desc.IsObject();
        desc.BoolField("skip_validation", skipValidation);
        desc.BoolField("force_dummy", forceDummy);
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
        desc.IsEnum();
        desc.Variant("alpha", isAlpha, alpha);
        desc.Variant("beta", isBeta, beta);
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
        desc.IsObject();
        desc.BoolField("unsafe", unsafe);
    }

    void Postprocess() override {
    }
};

struct LaunchConfig : public conf::Target {
    bool parallel = false;
    size_t threadCount = 8; // TODO
    size_t chunkSize = 10; // TODO

    void Describe(conf::Description& desc) override {
        desc.IsObject();
        desc.BoolField("parallel", parallel);
    }

    void Postprocess() override;
};
struct Config: public conf::Target {
    FilterConfig filter;
    EvalConfig eval;

    void Describe(conf::Description& desc) override {
        desc.IsObject();
        desc.ObjField("filter", filter);
        desc.ObjField("eval", eval);
    }

    void Postprocess() override {
    }
};
}
