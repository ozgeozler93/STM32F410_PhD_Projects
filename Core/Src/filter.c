#include "filter.h"
#include <math.h> // fabs() için gerekli

// Filtre algoritmasının kendisi
float apply_ema_filter(float current_val, float previous_val, float alpha) {
    return (alpha * current_val) + ((1.0f - alpha) * previous_val);
}


float apply_adaptive_ema_filter(float current_val, float previous_val) {
    float diff = fabs(current_val - previous_val);
    float alpha;

    // Değişim çok büyükse (örn: 1000 birim) bu muhtemelen anlık bir gölge veya kuş geçişidir.
    // Filtreyi sağırlaştır (eski değere güven)
    if(diff > 1000) {
        alpha = 0.05f;
    }
    // Değişim normalse (güneşin yavaş hareketi)
    else {
        alpha = 0.2f;
    }

    return (alpha * current_val) + ((1.0f - alpha) * previous_val);
}
