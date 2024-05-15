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


struct ShardingConfig : public conf::Target {
    struct RangeSharding : public conf::Target {
        size_t totalPartCount = 0;
        size_t rangeStart = 0;
        size_t rangeSize = 0;

        void Describe(conf::Description& desc) override {
            desc
                .Object()
                .NumField("total_part_count", totalPartCount)
                .NumField("range_start", rangeStart)
                .NumField("range_size", rangeSize);
        }

        void Postprocess() override;
    };

    bool isNone;
    RangeSharding range;
    bool isRange;

    void Describe(conf::Description& desc) override {
        desc
            .Enum()
            .SimpleVariant("none", isNone)
            .Variant("range", isRange, range);
    }

    void Postprocess() override {
    }
};

struct FilterConfig : public conf::Target {
    ShardingConfig sharding;
    bool enableIsomorphismCheck = true;

    void Describe(conf::Description& desc) override {
        desc
            .Object()
            .ObjField("sharding", sharding)
            .BoolField("check_isomorphism", enableIsomorphismCheck);
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
