#ifndef INC_FILTER_H_
#define INC_FILTER_H_

// Filtre fonksiyonunun prototipi
float apply_ema_filter(float current_val, float previous_val, float alpha);

// YENİ: Akıllı adaptif filtremizin prototipi
float apply_adaptive_ema_filter(float current_val, float previous_val);

#endif /* INC_FILTER_H_ */
