#pragma once

#include "conf/target.h"

#include <memory>

namespace bf {


struct EvalEngineConfig : public conf::Target {
    struct Alpha : public conf::Target {
        void Describe(conf::Description& desc) override {
            desc.Object();
        }

        void Postprocess() override {
        }
    };
    struct Beta : public conf::Target {
        struct PrintTopology : public conf::Target {
            bool enabled = false;
            bool wellKnown = false;
            int64_t x = 0;
            int64_t y = 0;

            void Describe(conf::Description& desc) override {
                desc.Object()
                    .BoolField("enabled", enabled)
                    .BoolField("well_known", wellKnown)
                    .NumField("x", x)
                    .NumField("y", y);
            }

            void Postprocess() override;
        };
        bool skipValidation = false;
        bool forceDummy = false;
        PrintTopology printTopology;

        void Describe(conf::Description& desc) override {
            desc.Object()
                .BoolField("skip_validation", skipValidation)
                .BoolField("force_dummy", forceDummy)
                .ObjField("print_topology", printTopology);
        }

        void Postprocess() override;
    };

    Alpha alpha;
    bool isAlpha = false;
    Beta beta;
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

struct SecondOutput : public conf::Target {
    struct Enabled : public conf::Target {
        int64_t x = 0;
        int64_t y = 0;

        void Describe(conf::Description& desc) override {
            desc.Object()
                .NumField("x", x)
                .NumField("y", y);
        }

        void Postprocess() override {
        }
    };

    bool isDisabled = false;
    Enabled enabled;
    bool isEnabled = false;

    void Describe(conf::Description& desc) override {
        desc.Enum()
            .SimpleVariant("disabled", isDisabled)
            .Variant("enabled", isEnabled, enabled);
    }

    void Postprocess() override {
    }
};

struct EvalConfig : public conf::Target {
    struct Settings : public conf::Target {
        SecondOutput secondOutput;
        uint8_t maxBits;

        void Describe(conf::Description& desc) override {
            desc.Object()
                .ObjField("second_output", secondOutput)
                .NumField("max_bits", maxBits);
        }

        void Postprocess() override {
        }
    };

    EvalEngineConfig engine;
    Settings settings;

    void Describe(conf::Description& desc) override {
        desc.Object()
            .ObjField("engine", engine)
            .ObjField("settings", settings);
    }

    void Postprocess() override {
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

    bool isNone = false;
    RangeSharding range;
    bool isRange = false;

    void Describe(conf::Description& desc) override {
        desc
            .Enum()
            .SimpleVariant("none", isNone)
            .Variant("range", isRange, range);
    }

    void Postprocess() override {
        if (!isNone && !isRange) {
            isNone = true;
        }
    }
};

struct FilterConfig : public conf::Target {
    ShardingConfig sharding;
    bool enableIsomorphismCheck = false;

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
