#ifndef DRP_CONFIDENCE_ENGINE_H
#define DRP_CONFIDENCE_ENGINE_H

#include <Arduino.h>
#include "config.h"

enum ClassificationState {
    STATE_CLEAR = 0,       // 0 - 24%
    STATE_UNCERTAIN = 1,   // 25 - 49%
    STATE_PROBABLE = 2,    // 50 - 74%
    STATE_CONFIRMED = 3    // 75 - 100%
};

struct FusionTelemetry {
    unsigned long timestampMs;
    float radarScore;
    float dopplerScore;
    float co2Score;
    float acousticScore;
    float orientationWeight;
    float rawComposite;
    float finalConfidence;
    ClassificationState classification;
    const char* classificationText;
};

class ConfidenceEngine {
public:
    ConfidenceEngine();
    
    FusionTelemetry compute(
        float radarPresenceScore,
        bool dopplerTriggerActive,
        float co2Score,
        float acousticScore,
        float orientationWeight
    );

    float getLatestConfidence() const { return _lastTelemetry.finalConfidence; }
    ClassificationState getLatestClassification() const { return _lastTelemetry.classification; }
    const char* getClassificationString(ClassificationState state) const;

private:
    FusionTelemetry _lastTelemetry;
    float _filteredConfidence;
    
    // Low pass alpha for smooth confidence transitions
    const float ALPHA = 0.25f;
};

#endif // DRP_CONFIDENCE_ENGINE_H
