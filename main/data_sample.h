
#ifndef DATA_SAMPLE_H
#define DATA_SAMPLE_H

struct AccSample_ {
  float ax, ay, az; //accelerations
  float mag_val; //magnetomenr
  float yaw, pitch, roll;
  
  float time;
};

typedef struct AccSample_ AccSample;

struct GPSSample_ {
  float lat, lon, h;
  bool have_signal;
  float speed_lat, speed_lon, speed_h;
  
  float time;
};

typedef struct GPSSample_ GPSSample;

struct LastSample_ {
    AccSample acc;
    GPSSample gps;
};

typedef struct LastSample_ LastSample;

extern LastSample last_sample;

#endif
