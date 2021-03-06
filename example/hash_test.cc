#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <inttypes.h>
#include <limits>

#include "nemo.h"
#include "xdebug.h"

using namespace nemo;

int main()
{
    nemo::Options options;
    options.target_file_size_base = 20 * 1024 * 1024;

    Nemo *n = new Nemo("./tmp/", options); 
    Status s;

    std::string res;

    /*
     *************************************************KV**************************************************
     */

    std::vector<std::string> keys;
    std::vector<KV> kvs;
    std::vector<KVS> kvss;
    std::vector<SM> sms;

    /*
     *************************************************HASH**************************************************
     */
    std::vector<std::string> fields;
    std::vector<std::string> values;
    std::vector<FV> fvs;
    std::vector<FVS> fvss;
    
    for (int i = 0; i < 2; i++) {
      /*
       *  Test HSet
       */
      log_info("======Test HSet======");
      s = n->HSet("tHSetKey", "field1", "value1");
      s = n->HSet("tHSetKey", "field2", "value2");
      s = n->HSet("tHSetKey", "field3", "value3");
      s = n->HSet("tHSetKey", "field4", "value4");
      s = n->HSet("tHSetKey", "field5", "value5");
      log_info("Test HSet OK return %s", s.ToString().c_str());
      log_info("");

      /*
       *  Test HGet
       */
      log_info("======Test HGet======");
      s = n->HSet("tHGetKey", "song", "tGetVal");
      res = "";
      s = n->HGet("tHGetKey", "song", &res);
      log_info("Test HGet OK return %s, result tHGetVal = %s", s.ToString().c_str(), res.c_str());
      res = "";
      s = n->HGet("tHGetNotFoundKey", "song", &res);
      log_info("Test Get NotFound return %s, result NULL = %s", s.ToString().c_str(), res.c_str());
      s = n->HGet("tHGetKey", "non-field", &res);
      log_info("Test Get NotFound return %s, result NULL = %s", s.ToString().c_str(), res.c_str());
      log_info("");

      /*
       *  Test HGetall
       */
      log_info("======Test HGetall======");
      fvs.clear();
      s = n->HGetall("tHSetKey", fvs);
      log_info("Test HGetall OK return %s", s.ToString().c_str());
      std::vector<FV>::iterator fv_iter;
      for (fv_iter = fvs.begin(); fv_iter != fvs.end(); fv_iter++) {
        log_info("Test HGetall, field: %s, val: %s", fv_iter->field.c_str(), fv_iter->val.c_str());
      }
      log_info("");

      /*
       *  Test HScan
       */
      log_info("======Test HScan 0 with Next======");

      HIterator *hit = n->HScan("tHSetKey", "", "", -1);
      if (hit == NULL) {
        log_info("HScan error!");
      }
      for (; hit->Valid(); hit->Next()) {
        log_info("HScan key: %s, field: %s, value: %s", hit->key().c_str(), hit->field().c_str(), hit->value().c_str());
      }
      log_info("");
      delete hit;

      //   log_info("======Test HScan 1 with Valid======");

      //   hit = n->HScan("tHSetKey", "field2", "field5", -1);
      //   if (hit == NULL) {
      //       log_info("HScan error!");
      //   }
      //   while (hit->Valid()) {
      //       log_info("HScan key: %s, field: %s, value: %s", hit->key().c_str(), hit->field().c_str(), hit->value().c_str());
      //       bool next_ret = hit->Next();
      //       log_info("  After hit->Next HScan key: %s, field: %s, value: %s", hit->key().c_str(), hit->field().c_str(), hit->value().c_str());
      //   }
      //   log_info("");

      /*
       *  Test HDelKey
       */
      log_info("======Test HDelKey======");

      int64_t del_ret;
      s = n->HDelKey("tHSetKey", &del_ret);
      log_info("Test HDelKey return %s", s.ToString().c_str());

      hit = n->HScan("tHSetKey", "", "", -1);
      if (hit == NULL) {
        log_info("HScan error!");
      }
      for (; hit->Valid(); hit->Next()) {
        log_info("HScan key: %s, field: %s, value: %s", hit->key().c_str(), hit->field().c_str(), hit->value().c_str());
      }
      log_info("");
      delete hit;
    }

    s = n->HSet("tHSetKey", "field11", "value11");
    s = n->HSet("tHSetKey", "field21", "value21");
    s = n->HSet("tHSetKey", "field31", "value31");
    s = n->HSet("tHSetKey", "field41", "value41");
    s = n->HSet("tHSetKey", "field51", "value51");

    /*
     *  Test HGetall
     */
    log_info("======Test HGetall with new fields======");
    fvs.clear();
    s = n->HGetall("tHSetKey", fvs);
    log_info("Test HGetall OK return %s", s.ToString().c_str());
    std::vector<FV>::iterator fv_iter;
    for (fv_iter = fvs.begin(); fv_iter != fvs.end(); fv_iter++) {
      log_info("Test HGetall, field: %s, val: %s", fv_iter->field.c_str(), fv_iter->val.c_str());
    }
    log_info("");

    /*
     *  Test HGetall
     */
    log_info("======Test Compact======");
    n->Compact();

    log_info("   ====== HGetall after compact======");
    fvs.clear();
    s = n->HGetall("tHSetKey", fvs);
    log_info("Test HGetall OK return %s", s.ToString().c_str());
    for (fv_iter = fvs.begin(); fv_iter != fvs.end(); fv_iter++) {
      log_info("Test HGetall, field: %s, val: %s", fv_iter->field.c_str(), fv_iter->val.c_str());
    }
    log_info("");

    /*
     *  Test Expire 
     */
    int64_t e_ret;
    int64_t ttl;

    log_info("======Test HExpire======");
    s = n->HSet("tHSetKey", "field11", "value11");
    s = n->HExpire("tHSetKey", 7, &e_ret);
    log_info("Test HExpire with key=tHSetKey in 7s, return %s", s.ToString().c_str());

    for (int i = 0; i < 3; i++) {
        sleep(3);
        s = n->HGet("tHSetKey", "field11", &res);
        log_info("          after %ds, return %s", (i+1)*3, s.ToString().c_str());
        if (s.ok()) {
            n->HTTL("tHSetKey", &ttl);
            log_info("          new TTL is %ld, HGet field11 res:%s\n", ttl, res.c_str());
        }
    }
    log_info("");

    /*
     *  Test Expireat
     */
    log_info("======Test HExpireat======");
    s = n->HSet("tHSetKey", "field12", "value11");

    std::time_t t = std::time(0);
    s = n->HExpireat("tHSetKey", t + 8, &e_ret);
    log_info("Test Expireat with key=tHSetKey at timestamp=%ld in 8s, return %s", (t+8), s.ToString().c_str());

    for (int i = 0; i < 3; i++) {
        sleep(3);
        s = n->HGet("tHSetKey", "field12", &res);
        log_info("          after %ds, return %s", (i+1)*3, s.ToString().c_str());
        if (s.ok()) {
            n->HTTL("tHSetKey", &ttl);
            log_info("          new TTL is %ld, HGet field12 res:%s\n", ttl, res.c_str());
        }
    }
    log_info("");

    s = n->HSet("tHSetKey", "field12", "value11");
    s = n->HExpireat("tHSetKey", 8, &e_ret);
    log_info("Test HExpireat with key=tHSetKey at a passed timestamp=8, return %s", s.ToString().c_str());
    s = n->HGet("tHSetKey", "field12", &res);
    log_info("          Get a invalid key return %s, expect ok",  s.ToString().c_str());
    if (s.IsNotFound()) {
        n->TTL("tHSetKey", &ttl);
        log_info("          NotFound key's TTL is %ld, HGet res:%s\n", ttl, res.c_str());
    }
    log_info("");

    /*
     *  Test Persist 
     */
    log_info("======Test HPersist======");
    s = n->HSet("tHSetKey", "field12", "value11");
    s = n->HExpire("tHSetKey", 7, &e_ret);
    log_info("Test Persist with key=tHSetKey in 7s, return %s", s.ToString().c_str());

    for (int i = 0; i < 3; i++) {
        sleep(3);
        if (i == 1) {
            s = n->HPersist("tHSetKey", &e_ret);
            log_info(" Test HPersist return %s", s.ToString().c_str());
        }
        s = n->HGet("tHSetKey", "field12", &res);
        log_info("          after %ds, return %s", (i+1)*3, s.ToString().c_str());
        if (s.ok()) {
            n->HTTL("tHSetKey", &ttl);
            log_info("          new TTL is %ld, HGet field12 res:%s\n", ttl, res.c_str());
        }
    }
    log_info("");

    delete n;

    return 0;
}
