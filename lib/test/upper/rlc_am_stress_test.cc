/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2015 Software Radio Systems Limited
 *
 * \section LICENSE
 *
 * This file is part of the srsUE library.
 *
 * srsUE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsUE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include "srslte/common/log_filter.h"
#include "srslte/common/logger_stdout.h"
#include "srslte/common/threads.h"
#include "srslte/common/rlc_pcap.h"
#include "srslte/upper/rlc.h"
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <assert.h>
#include <srslte/upper/rlc_interface.h>

#define SDU_SIZE 1500

using namespace std;
using namespace srsue;
using namespace srslte;
namespace bpo = boost::program_options;

typedef struct {
  std::string mode;
  uint32_t    test_duration_sec;
  float       error_rate;
  uint32_t    sdu_gen_delay_usec;
  uint32_t    pdu_tx_delay_usec;
  bool        reestablish;
  uint32_t    log_level;
  bool        single_tx;
  bool        write_pcap;
  float       opp_sdu_ratio;
} stress_test_args_t;

void parse_args(stress_test_args_t *args, int argc, char *argv[]) {

  // Command line only options
  bpo::options_description general("General options");

  general.add_options()
  ("help,h", "Produce help message")
  ("version,v", "Print version information and exit");

  // Command line or config file options
  bpo::options_description common("Configuration options");
  common.add_options()
  ("mode",          bpo::value<std::string>(&args->mode)->default_value("AM"), "Whether to test RLC acknowledged or unacknowledged mode (AM/UM)")
  ("duration",      bpo::value<uint32_t>(&args->test_duration_sec)->default_value(10), "Duration (sec)")
  ("sdu_gen_delay", bpo::value<uint32_t>(&args->sdu_gen_delay_usec)->default_value(10), "SDU generation delay (usec)")
  ("pdu_tx_delay",  bpo::value<uint32_t>(&args->pdu_tx_delay_usec)->default_value(10), "Delay in MAC for transfering PDU from tx'ing RLC to rx'ing RLC (usec)")
  ("error_rate",    bpo::value<float>(&args->error_rate)->default_value(0.1), "Rate at which RLC PDUs are dropped")
  ("opp_sdu_ratio", bpo::value<float>(&args->opp_sdu_ratio)->default_value(0.0), "Ratio between MAC opportunity and SDU size (0==random)")
  ("reestablish",   bpo::value<bool>(&args->reestablish)->default_value(false), "Mimic RLC reestablish during execution")
  ("loglevel",      bpo::value<uint32_t>(&args->log_level)->default_value(srslte::LOG_LEVEL_DEBUG), "Log level (1=Error,2=Warning,3=Info,4=Debug)")
  ("singletx",      bpo::value<bool>(&args->single_tx)->default_value(false), "If set to true, only one node is generating data")
  ("pcap",          bpo::value<bool>(&args->write_pcap)->default_value(false), "Whether to write all RLC PDU to PCAP file");

  // these options are allowed on the command line
  bpo::options_description cmdline_options;
  cmdline_options.add(common).add(general);

  // parse the command line and store result in vm
  bpo::variables_map vm;
  bpo::store(bpo::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
  bpo::notify(vm);

  // help option was given - print usage and exit
  if (vm.count("help")) {
    cout << "Usage: " << argv[0] << " [OPTIONS] config_file" << endl << endl;
    cout << common << endl << general << endl;
    exit(0);
  }

  if (args->log_level > 4) {
    args->log_level = 4;
    printf("Set log level to %d (%s)\n", args->log_level, srslte::log_level_text[args->log_level]);
  }
}

class mac_reader
    :public thread
{
public:
  mac_reader(rlc_interface_mac *rlc1_, rlc_interface_mac *rlc2_, float fail_rate_, float opp_sdu_ratio_, uint32_t pdu_tx_delay_usec_, rlc_pcap *pcap_, bool is_dl_ = true)
  {
    rlc1 = rlc1_;
    rlc2 = rlc2_;
    fail_rate = fail_rate_;
    opp_sdu_ratio = opp_sdu_ratio_;
    run_enable = true;
    running = false;
    pdu_tx_delay_usec = pdu_tx_delay_usec_;
    pcap = pcap_;
    is_dl = is_dl_;
  }

  void stop()
  {
    run_enable = false;
    int cnt=0;
    while(running && cnt<100) {
      usleep(10000);
      cnt++;
    }
    if(running) {
      thread_cancel();
    }
    wait_thread_finish();
  }

private:
  void run_thread()
  {
    running = true;
    byte_buffer_t *pdu = byte_buffer_pool::get_instance()->allocate("mac_reader::run_thread");
    if (!pdu) {
      printf("Fatal Error: Could not allocate PDU in mac_reader::run_thread\n");
      exit(-1);
    }

    while(run_enable) {
      // generate MAC opportunities of random size or with fixed ratio
      float r = opp_sdu_ratio ? opp_sdu_ratio : (float)rand()/RAND_MAX;
      int opp_size = r*SDU_SIZE;
      uint32_t buf_state = rlc1->get_buffer_state(1);
      if (buf_state) {
        int read = rlc1->read_pdu(1, pdu->msg, opp_size);
        usleep(pdu_tx_delay_usec);
        if(((float)rand()/RAND_MAX > fail_rate) && read>0) {
          pdu->N_bytes = read;
          rlc2->write_pdu(1, pdu->msg, pdu->N_bytes);
          if (is_dl) {
            pcap->write_dl_am_ccch(pdu->msg, pdu->N_bytes);
          } else {
            pcap->write_ul_am_ccch(pdu->msg, pdu->N_bytes);
          }
        }
      }
    }
    running = false;
    byte_buffer_pool::get_instance()->deallocate(pdu);
  }

  rlc_interface_mac *rlc1;
  rlc_interface_mac *rlc2;
  float fail_rate;
  float opp_sdu_ratio;
  uint32_t pdu_tx_delay_usec;
  rlc_pcap *pcap;
  bool is_dl;

  bool run_enable;
  bool running;
};

class mac_dummy
    :public srslte::mac_interface_timers
{
public:
  mac_dummy(rlc_interface_mac *rlc1_, rlc_interface_mac *rlc2_, float fail_rate_, float opp_sdu_ratio_, int32_t pdu_tx_delay, rlc_pcap* pcap = NULL)
    :r1(rlc1_, rlc2_, fail_rate_, opp_sdu_ratio_, pdu_tx_delay, pcap, true)
    ,r2(rlc2_, rlc1_, fail_rate_, opp_sdu_ratio_, pdu_tx_delay, pcap, false)
  {
  }

  void start()
  {
    r1.start(7);
    r2.start(7);
  }

  void stop()
  {
    r1.stop();
    r2.stop();
  }

  srslte::timers::timer* timer_get(uint32_t timer_id)
  {
    return &t;
  }
  uint32_t timer_get_unique_id(){return 0;}
  void timer_release_id(uint32_t id){}

private:
  srslte::timers::timer t;

  mac_reader r1;
  mac_reader r2;
};



class rlc_tester
    :public pdcp_interface_rlc
    ,public rrc_interface_rlc
    ,public thread
{
public:
  rlc_tester(rlc_interface_pdcp *rlc_, std::string name_, uint32_t sdu_gen_delay_usec_){
    rlc = rlc_;
    run_enable = true;
    running = false;
    rx_pdus = 0;
    name = name_;
    sdu_gen_delay_usec = sdu_gen_delay_usec_;
  }

  void stop()
  {
    run_enable = false;
    int cnt=0;
    while(running && cnt<100) {
      usleep(10000);
      cnt++;
    }
    if(running) {
      thread_cancel();
    }
    wait_thread_finish();
  }

  // PDCP interface
  void write_pdu(uint32_t lcid, byte_buffer_t *sdu)
  {
    assert(lcid == 1);
    assert(sdu->N_bytes==SDU_SIZE);
    byte_buffer_pool::get_instance()->deallocate(sdu);
    std::cout << "rlc_tester " << name << " received " << rx_pdus++ << " PDUs" << std::endl;
  }
  void write_pdu_bcch_bch(byte_buffer_t *sdu) {}
  void write_pdu_bcch_dlsch(byte_buffer_t *sdu) {}
  void write_pdu_pcch(byte_buffer_t *sdu) {}
  
  // RRC interface
  void max_retx_attempted(){}
  std::string get_rb_name(uint32_t lcid) { return std::string(""); }

private:
  void run_thread()
  {
    uint8_t sn = 0;
    running = true;
    while(run_enable) {
      byte_buffer_t *pdu = byte_buffer_pool::get_instance()->allocate("rlc_tester::run_thread");
      if (!pdu) {
        printf("Fatal Error: Could not allocate PDU in rlc_tester::run_thread\n");
        exit(-1);
      }
      for (uint32_t i = 0; i < SDU_SIZE; i++) {
        pdu->msg[i] = sn;
      }
      sn++;
      pdu->N_bytes = SDU_SIZE;
      rlc->write_sdu(1, pdu);
      usleep(sdu_gen_delay_usec);
    }
    running = false;
  }

  bool run_enable;
  bool running;
  long rx_pdus;

  std::string name;

  uint32_t sdu_gen_delay_usec;

  rlc_interface_pdcp *rlc;
};

void stress_test(stress_test_args_t args)
{
  srslte::log_filter log1("RLC_1");
  srslte::log_filter log2("RLC_2");
  log1.set_level((LOG_LEVEL_ENUM)args.log_level);
  log2.set_level((LOG_LEVEL_ENUM)args.log_level);
  log1.set_hex_limit(-1);
  log2.set_hex_limit(-1);
  rlc_pcap pcap;

  if (args.write_pcap) {
    pcap.open("rlc_stress_test.pcap", 0);
  }

  rlc rlc1;
  rlc rlc2;

  rlc_tester tester1(&rlc1, "tester1", args.sdu_gen_delay_usec);
  rlc_tester tester2(&rlc2, "tester2", args.sdu_gen_delay_usec);
  mac_dummy     mac(&rlc1, &rlc2, args.error_rate, args.opp_sdu_ratio, args.pdu_tx_delay_usec, &pcap);
  ue_interface  ue;

  rlc1.init(&tester1, &tester1, &ue, &log1, &mac, 0);
  rlc2.init(&tester2, &tester2, &ue, &log2, &mac, 0);

  srslte_rlc_config_t cnfg_;
  if (args.mode == "AM") {
    // config RLC AM bearer
    cnfg_.rlc_mode = LIBLTE_RRC_RLC_MODE_AM;
    cnfg_.am.max_retx_thresh = 4;
    cnfg_.am.poll_byte = 25*1000;
    cnfg_.am.poll_pdu = 4;
    cnfg_.am.t_poll_retx = 5;
    cnfg_.am.t_reordering = 5;
    cnfg_.am.t_status_prohibit = 5;
  } else if (args.mode == "UM") {
    // config UM bearer
    cnfg_.rlc_mode = LIBLTE_RRC_RLC_MODE_UM_BI;
    cnfg_.um.t_reordering = 5;
    cnfg_.um.rx_mod = 32;
    cnfg_.um.rx_sn_field_length = RLC_UMD_SN_SIZE_5_BITS;
    cnfg_.um.rx_window_size = 16;
    cnfg_.um.tx_sn_field_length = RLC_UMD_SN_SIZE_5_BITS;
    cnfg_.um.tx_mod = 32;
  } else {
    cout << "Unsupported RLC mode " << args.mode << ", exiting." << endl;
    return;
  }

  rlc1.add_bearer(1, cnfg_);
  rlc2.add_bearer(1, cnfg_);

  tester1.start(7);
  if (!args.single_tx) {
    tester2.start(7);
  }
  mac.start();

  for (uint32_t i = 0; i < args.test_duration_sec; i++) {
    // if enabled, mimic reestablishment every second
    if (args.reestablish) {
      rlc1.reestablish();
      rlc2.reestablish();
    }
    usleep(1e6);
  }

  tester1.stop();
  tester2.stop();
  mac.stop();
  if (args.write_pcap) {
    pcap.close();
  }
}


int main(int argc, char **argv) {
  stress_test_args_t args;
  parse_args(&args, argc, argv);

  stress_test(args);
  byte_buffer_pool::get_instance()->cleanup();
}
