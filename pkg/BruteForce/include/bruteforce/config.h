#pragma once

#include "conf/conf.h"

#include <memory>

namespace bf {
struct OutputAlphaConfig: public conf::Target {
    void Describe(conf::Description& desc) override {
        desc.IsObject();
    }
};
struct OutputConfig : public conf::Target {
    OutputAlphaConfig alpha;
    bool isAlpha;

    void Describe(conf::Description& desc) override {
        desc.IsEnum();
        desc.Variant("alpha", isAlpha, alpha);
    }
};
struct Config: public conf::Target {
    bool unsafe;
    OutputConfig output;

    void Describe(conf::Description& desc) override {
        desc.IsObject();
        desc.BoolField("unsafe", unsafe);
        desc.ObjField("output", output);
    }
};
}