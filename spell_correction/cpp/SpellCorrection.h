/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 */
#ifndef SpellCorrection_H
#define SpellCorrection_H

#include <TProcessor.h>
#include "spell_correction_types.h"

namespace Adingo {

class SpellCorrectionIf {
 public:
  virtual ~SpellCorrectionIf() {}
  virtual void correctme(std::vector<std::vector<std::string> > & _return, const std::vector<std::string> & queries) = 0;
};

class SpellCorrectionNull : virtual public SpellCorrectionIf {
 public:
  virtual ~SpellCorrectionNull() {}
  void correctme(std::vector<std::vector<std::string> > & /* _return */, const std::vector<std::string> & /* queries */) {
    return;
  }
};

class SpellCorrection_correctme_args {
 public:

  SpellCorrection_correctme_args() {
  }

  virtual ~SpellCorrection_correctme_args() throw() {}

  std::vector<std::string>  queries;

  struct __isset {
    __isset() : queries(false) {}
    bool queries;
  } __isset;

  bool operator == (const SpellCorrection_correctme_args & rhs) const
  {
    if (!(queries == rhs.queries))
      return false;
    return true;
  }
  bool operator != (const SpellCorrection_correctme_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpellCorrection_correctme_args & ) const;

  uint32_t read(apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(apache::thrift::protocol::TProtocol* oprot) const;

};

class SpellCorrection_correctme_pargs {
 public:


  virtual ~SpellCorrection_correctme_pargs() throw() {}

  const std::vector<std::string> * queries;

  uint32_t write(apache::thrift::protocol::TProtocol* oprot) const;

};

class SpellCorrection_correctme_result {
 public:

  SpellCorrection_correctme_result() {
  }

  virtual ~SpellCorrection_correctme_result() throw() {}

  std::vector<std::vector<std::string> >  success;

  struct __isset {
    __isset() : success(false) {}
    bool success;
  } __isset;

  bool operator == (const SpellCorrection_correctme_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SpellCorrection_correctme_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpellCorrection_correctme_result & ) const;

  uint32_t read(apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(apache::thrift::protocol::TProtocol* oprot) const;

};

class SpellCorrection_correctme_presult {
 public:


  virtual ~SpellCorrection_correctme_presult() throw() {}

  std::vector<std::vector<std::string> > * success;

  struct __isset {
    __isset() : success(false) {}
    bool success;
  } __isset;

  uint32_t read(apache::thrift::protocol::TProtocol* iprot);

};

class SpellCorrectionClient : virtual public SpellCorrectionIf {
 public:
  SpellCorrectionClient(boost::shared_ptr<apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  SpellCorrectionClient(boost::shared_ptr<apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr<apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr<apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr<apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void correctme(std::vector<std::vector<std::string> > & _return, const std::vector<std::string> & queries);
  void send_correctme(const std::vector<std::string> & queries);
  void recv_correctme(std::vector<std::vector<std::string> > & _return);
 protected:
  boost::shared_ptr<apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr<apache::thrift::protocol::TProtocol> poprot_;
  apache::thrift::protocol::TProtocol* iprot_;
  apache::thrift::protocol::TProtocol* oprot_;
};

class SpellCorrectionProcessor : virtual public apache::thrift::TProcessor {
 protected:
  boost::shared_ptr<SpellCorrectionIf> iface_;
  virtual bool process_fn(apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, std::string& fname, int32_t seqid);
 private:
  std::map<std::string, void (SpellCorrectionProcessor::*)(int32_t, apache::thrift::protocol::TProtocol*, apache::thrift::protocol::TProtocol*)> processMap_;
  void process_correctme(int32_t seqid, apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot);
 public:
  SpellCorrectionProcessor(boost::shared_ptr<SpellCorrectionIf> iface) :
    iface_(iface) {
    processMap_["correctme"] = &SpellCorrectionProcessor::process_correctme;
  }

  virtual bool process(boost::shared_ptr<apache::thrift::protocol::TProtocol> piprot, boost::shared_ptr<apache::thrift::protocol::TProtocol> poprot);
  virtual ~SpellCorrectionProcessor() {}
};

class SpellCorrectionMultiface : virtual public SpellCorrectionIf {
 public:
  SpellCorrectionMultiface(std::vector<boost::shared_ptr<SpellCorrectionIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~SpellCorrectionMultiface() {}
 protected:
  std::vector<boost::shared_ptr<SpellCorrectionIf> > ifaces_;
  SpellCorrectionMultiface() {}
  void add(boost::shared_ptr<SpellCorrectionIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void correctme(std::vector<std::vector<std::string> > & _return, const std::vector<std::string> & queries) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        ifaces_[i]->correctme(_return, queries);
        return;
      } else {
        ifaces_[i]->correctme(_return, queries);
      }
    }
  }

};

} // namespace

#endif
