#pragma once

#include "conf/conf.h"

#include <memory>

namespace bf {
struct OutputAlphaConfig: public conf::Target {
    void Describe(conf::Description& desc) override {
        desc.IsObject();
    }

    void Postprocess() override {
    }
};
struct OutputBetaConfig : public conf::Target {
    bool skipValidation = false;

    void Describe(conf::Description& desc) override {
        desc.IsObject();
        desc.BoolField("skip_validation", skipValidation);
    }

    void Postprocess() override {
        if (skipValidation) {
            throw conf::BindingException("TODO");
        }
    }
};
struct OutputConfig : public conf::Target {
    OutputAlphaConfig alpha;
    bool isAlpha = false;
    OutputBetaConfig beta;
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
    bool unsafe;

    void Describe(conf::Description& desc) override {
        desc.IsObject();
        desc.BoolField("unsafe", unsafe);
    }

    void Postprocess() override {
    }
};
struct Config: public conf::Target {
    FilterConfig filter;
    OutputConfig output;

    void Describe(conf::Description& desc) override {
        desc.IsObject();
        desc.ObjField("filter", filter);
        desc.ObjField("output", output);
    }

    void Postprocess() override {
    }
};
}
