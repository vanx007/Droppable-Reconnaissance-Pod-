#include "fusion/confidence_engine.h"

ConfidenceEngine::ConfidenceEngine() : _filteredConfidence(0.0f) {
    memset(&_lastTelemetry, 0, sizeof(FusionTelemetry));
    _lastTelemetry.classificationText = "CLEAR / NO TARGET";
}

const char* ConfidenceEngine::getClassificationString(ClassificationState state) const {
    switch (state) {
        case STATE_CLEAR:     return "CLEAR / NO TARGET";
        case STATE_UNCERTAIN: return "UNCERTAIN / AMBIENT NOISE";
        case STATE_PROBABLE:  return "PROBABLE OCCUPANT";
        case STATE_CONFIRMED: return "CONFIRMED HUMAN PRESENCE";
        default:              return "UNKNOWN";
    }
}

FusionTelemetry ConfidenceEngine::compute(
    float radarPresenceScore,
    bool dopplerTriggerActive,
    float co2Score,
    float acousticScore,
    float orientationWeight
) {
    FusionTelemetry result;
    result.timestampMs = millis();
    result.radarScore = radarPresenceScore;
    result.dopplerScore = dopplerTriggerActive ? 100.0f : 0.0f;
    result.co2Score = co2Score;
    result.acousticScore = acousticScore;
    result.orientationWeight = orientationWeight;

    // Multi-modal weighted combination
    float composite = (WEIGHT_RADAR    * radarPresenceScore) +
                      (WEIGHT_DOPPLER  * result.dopplerScore) +
                      (WEIGHT_CO2      * co2Score) +
                      (WEIGHT_ACOUSTIC * acousticScore);

    result.rawComposite = composite;

    // Apply geometric tilt orientation penalty: C = W_theta * Composite
    float orientedScore = composite * orientationWeight;

    // Temporal smoothing (IIR filter) to prevent single-cycle jitter
    _filteredConfidence = (_filteredConfidence * (1.0f - ALPHA)) + (orientedScore * ALPHA);
    
    // Clamp to 0.0 - 100.0%
    if (_filteredConfidence < 0.0f) _filteredConfidence = 0.0f;
    if (_filteredConfidence > 100.0f) _filteredConfidence = 100.0f;

    result.finalConfidence = _filteredConfidence;

    // Determine Classification Band with Hysteresis
    if (result.finalConfidence >= CONFIDENCE_BAND_CONFIRMED) {
        result.classification = STATE_CONFIRMED;
    } else if (result.finalConfidence >= CONFIDENCE_BAND_PROBABLE) {
        result.classification = STATE_PROBABLE;
    } else if (result.finalConfidence >= CONFIDENCE_BAND_UNCERTAIN) {
        result.classification = STATE_UNCERTAIN;
    } else {
        result.classification = STATE_CLEAR;
    }

    result.classificationText = getClassificationString(result.classification);
    _lastTelemetry = result;
    return result;
}
