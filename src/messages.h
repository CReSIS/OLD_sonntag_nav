#pragma once

#include <cstdint>

struct __attribute__((__packed__)) Header
{
  uint8_t sync[3];
  uint8_t header_len;
  uint16_t message_id;
  uint8_t message_type;
  uint8_t port_addr;
  uint16_t message_len;
  uint16_t sequence_id;
  uint8_t idle_time;
  uint8_t time_status;
  uint16_t week;
  uint32_t ms;
  uint32_t rx_stat;
  uint16_t _;
  uint16_t version;
};

struct __attribute__((__packed__)) BestPos
{
  uint32_t sol_stat;
  uint32_t pos_type;
  double latitude;
  double longitude;
  double height;
  float undulation;
  uint32_t datuum_id;
  float lat_stdev;
  float long_stdev;
  float hgt_stdev;
  int8_t station_id[4];
  float diff_age;
  float sol_age;
  uint8_t tracked_sats;
  uint8_t solution_sats;
  uint8_t sol_L1_sats;
  uint8_t sol_multi_sats;
  uint8_t _;
  uint8_t ext_sol_stat;
  uint8_t gal_bei_mask;
  uint8_t gps_glon_mask;
};

struct __attribute__((__packed__)) BestVel
{
  uint32_t sol_stat;
  uint32_t vel_type;
  float latency;
  float age;
  double hor_spd;
  double trk_gnd;
  double vert_spd;
  float _;
};

struct __attribute__((__packed__)) Time
{
  uint32_t clk_stat;
  double offset;
  double offset_stdev;
  double utc_offset;
  uint32_t utc_year;
  uint8_t utc_month;
  uint8_t utc_day;
  uint8_t utc_hour;
  uint8_t utc_min;
  uint32_t utc_ms;
  uint32_t utc_stat;
};