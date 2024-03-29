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
  uint32_t sol_stat = 0;
  uint32_t pos_type = 0;
  double latitude = 0;
  double longitude = 0;
  double height = 0;
  float undulation = 0;
  uint32_t datuum_id = 0;
  float lat_stdev = 0;
  float long_stdev = 0;
  float hgt_stdev = 0;
  int8_t station_id[4];
  float diff_age = 0;
  float sol_age = 0;
  uint8_t tracked_sats = 0;
  uint8_t solution_sats = 0;
  uint8_t sol_L1_sats = 0;
  uint8_t sol_multi_sats = 0;
  uint8_t _;
  uint8_t ext_sol_stat = 0;
  uint8_t gal_bei_mask = 0;
  uint8_t gps_glon_mask = 0;
};

struct __attribute__((__packed__)) BestVel
{
  uint32_t sol_stat = 0;
  uint32_t vel_type = 0;
  float latency = 0;
  float age = 0;
  double hor_spd = 0;
  double trk_gnd = 0;
  double vert_spd = 0;
  float _;
};

struct __attribute__((__packed__)) BestTime
{
  uint32_t clk_stat = 0;
  double offset = 0.0;
  double offset_stdev = 0;
  double utc_offset = 0;
  uint32_t utc_year = 0;
  uint8_t utc_month = 0;
  uint8_t utc_day = 0;
  uint8_t utc_hour = 0;
  uint8_t utc_min = 0;
  uint32_t utc_ms = 0;
  uint32_t utc_stat = 0;
};

struct __attribute__((__packed__)) Heading
{
  uint32_t sol_stat = 0;
  uint32_t pos_type = 0;
  float length = 0.0;
  float heading = 0.0;
  float pitch = 0.0;
  float reserved = 0.0;
  float heading_stdev= 0.0;
  float pitch_stdev= 0.0;
  int8_t station_id[4];
  uint8_t tracked_sats = 0;
  uint8_t solution_sats = 0;
  uint8_t num_sat_above_elev_mask = 0;
  uint8_t num_sat_above_elev_mask_with_L2 = 0;
  uint8_t sol_source = 0;
  uint8_t ext_sol_stat = 0;
  uint8_t gal_bei_mask = 0;
  uint8_t gps_glon_mask = 0;
};

struct __attribute__((__packed__)) InsAtt
{
  uint32_t gnss_week= 0;
  double gnss_sec = 0;
  double roll = 0.0;
  double pitch = 0.0;
  double azimuth = 0.0;
  uint32_t status = 0;
};
