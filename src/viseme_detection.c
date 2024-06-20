#include <emscripten.h>
#include <math.h>

#define SENSITIVITY_THRESHOLD 0.5
#define FFT_SIZE 1024
#define SAMPLING_FREQUENCY 44100

EMSCRIPTEN_KEEPALIVE
void detectVisemes(float *spectrum, int spectrum_length, float *visemes) {
    float boundingFrequencyLow[] = {0, 400, 560, 2400, 4800};
    float boundingFrequencyHigh[] = {0, 500, 700, 3000, 6000};
    int indicesFrequencyLow[5], indicesFrequencyHigh[5];
    float energyBinLow[5] = {0}, energyBinHigh[5] = {0};
    float stPSD[spectrum_length];

    for (int i = 0; i < 5; ++i) {
        indicesFrequencyLow[i] = round((2.0 * FFT_SIZE / SAMPLING_FREQUENCY) * boundingFrequencyLow[i]);
        indicesFrequencyHigh[i] = round((2.0 * FFT_SIZE / SAMPLING_FREQUENCY) * boundingFrequencyHigh[i]);
    }

    for (int i = 0; i < spectrum_length; ++i) {
        stPSD[i] = SENSITIVITY_THRESHOLD + (spectrum[i] + 20) / 140.0;
    }

    for (int m = 0; m < 4; ++m) {
        for (int j = indicesFrequencyLow[m]; j <= indicesFrequencyLow[m + 1]; ++j) {
            if (stPSD[j] > 0) energyBinLow[m] += stPSD[j];
        }
        energyBinLow[m] /= (indicesFrequencyLow[m + 1] - indicesFrequencyLow[m]);
    }

    for (int m = 0; m < 4; ++m) {
        for (int j = indicesFrequencyHigh[m]; j <= indicesFrequencyHigh[m + 1]; ++j) {
            if (stPSD[j] > 0) energyBinHigh[m] += stPSD[j];
        }
        energyBinHigh[m] /= (indicesFrequencyHigh[m + 1] - indicesFrequencyHigh[m]);
    }

    visemes[0] = fmax(energyBinHigh[1], energyBinLow[1]) > 0.2
                 ? 1 - 2 * fmax(energyBinLow[2], energyBinHigh[2])
                 : (1 - 2 * fmax(energyBinLow[2], energyBinHigh[2])) * 5 * fmax(energyBinLow[1], energyBinHigh[1]);

    visemes[1] = 3 * fmax(energyBinLow[3], energyBinHigh[3]);
    visemes[2] = 0.8 * (fmax(energyBinLow[1], energyBinHigh[1]) - fmax(energyBinLow[3], energyBinHigh[3]));
}

EMSCRIPTEN_KEEPALIVE
const char* classifyPhoneme(float oh, float ah, float ee) {
    float sum = oh + ah + ee;
    if (sum < 0.05) return "quiet";
    float rel_oh = oh / sum;
    float rel_ah = ah / sum;
    float rel_ee = ee / sum;

    if (rel_oh > 0.3 && rel_ah < 0.04 && rel_ee < 0.15) return "oo";
    if (rel_oh > 0.3 && rel_ah < 0.05 && rel_ee >= 0.15) return "oh_ah";
    if (rel_oh > 0.3 && rel_ah > 0.05 && rel_ee > 0.2) return "eh";
    if (rel_oh > 0.35 && rel_ah > 0.05 && rel_ee > 0.1) return "ee";
    if (rel_oh < 0.3 && rel_ah > 0.4 && rel_ee < 0.1) return "sh";
    if (rel_oh > 0.3 && rel_ah > 0.25 && rel_ee < 0.05) return "f";
    if (rel_oh > 0.3 && rel_ah > 0.15 && rel_ee < 0.05) return "v";
    return "unknown";
}
