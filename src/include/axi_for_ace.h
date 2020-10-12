/*
 * Copyright (c) 2017-2019, NVIDIA CORPORATION.  All rights reserved.
 * 
 * Modifications Copyright (c) 2019-2020 Integrated Circuits Lab, Democritus University of Thrace, Greece.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _AXI_FOR_ACE_H_
#define _AXI_FOR_ACE_H_

#include <systemc>
#include <nvhls_connections.h>
#include <nvhls_assert.h>
#include <nvhls_message.h>
#include <nvhls_module.h>

#include <UIntOrEmpty.h>

#include <axi/axi4_encoding.h>
#include <axi/axi4_configs.h>

/**
 * \brief The ace namespace contains classes and definitions related to the AXI standard.
 * \ingroup AXI
 */
namespace ace {

template <typename Cfg>
class axi4 {
 public:
  typedef axi::AXI4_Encoding Enc;

  enum {
    DATA_WIDTH = Cfg::dataWidth,
    ADDR_WIDTH = Cfg::addrWidth,
    ID_WIDTH = Cfg::idWidth,
    BID_WIDTH = (Cfg::useWriteResponses == 0 ? 0 : Cfg::idWidth),
    ALEN_WIDTH =
        (Cfg::useBurst != 0 ? nvhls::log2_ceil<Cfg::maxBurstSize>::val : 0),
    ASIZE_WIDTH = (Cfg::useVariableBeatSize != 0 ? 3 : 0),
    LAST_WIDTH = (Cfg::useLast != 0 ? 1 : 0),
    CACHE_WIDTH = (Cfg::useCache != 0 ? Enc::ARCACHE::_WIDTH : 0),
    BURST_WIDTH = ((Cfg::useBurst != 0 &&
                    (Cfg::useFixedBurst != 0 || Cfg::useWrapBurst != 0))
                       ? Enc::AXBURST::_WIDTH
                       : 0),
    WSTRB_WIDTH = (Cfg::useWriteStrobes != 0 ? (DATA_WIDTH >> 3) : 0),
    RESP_WIDTH = Cfg::useACE ? Enc::XRESP::_WIDTH+2 : Enc::XRESP::_WIDTH,
    // TODO - The B channel ought to disappear entirely if useWriteResponses is
    // 0, but that will require substantial refactoring.  For now we leave
    // RESP_WIDTH so there is a data stub in the ready-valid interface.

    AUSER_WIDTH = Cfg::aUserWidth,
    WUSER_WIDTH = Cfg::wUserWidth,
    BUSER_WIDTH = (Cfg::useWriteResponses == 0 ? 0 : Cfg::bUserWidth),
    RUSER_WIDTH = Cfg::rUserWidth,
    
    C_SNOOP_WIDTH   = Cfg::useACE ? 4 : 0,
    C_DOMAIN_WIDTH  = Cfg::useACE ? 2 : 0,
    C_BARRIER_WIDTH = Cfg::useACE ? 2 : 0,
    C_UNIQUE_WIDTH  = Cfg::useACE ? 1 : 0, // The AWUNIQUE signal is only required by a component that supports the WriteEvict transaction.
    
  };

  typedef NVUINTW(ADDR_WIDTH) Addr;
  typedef NVUINTW(DATA_WIDTH) Data;
  typedef typename nvhls::UIntOrEmpty<ID_WIDTH>::T Id;
  typedef typename nvhls::UIntOrEmpty<BID_WIDTH>::T BId;
  typedef typename nvhls::UIntOrEmpty<ALEN_WIDTH>::T BeatNum;
  typedef typename nvhls::UIntOrEmpty<ASIZE_WIDTH>::T BeatSize;
  typedef typename nvhls::UIntOrEmpty<LAST_WIDTH>::T Last;
  typedef typename nvhls::UIntOrEmpty<WSTRB_WIDTH>::T Wstrb;
  typedef typename nvhls::UIntOrEmpty<CACHE_WIDTH>::T Cache;
  typedef typename nvhls::UIntOrEmpty<BURST_WIDTH>::T Burst;
  typedef NVUINTW(RESP_WIDTH) Resp;

  typedef typename nvhls::UIntOrEmpty<AUSER_WIDTH>::T AUser;
  typedef typename nvhls::UIntOrEmpty<WUSER_WIDTH>::T WUser;
  typedef typename nvhls::UIntOrEmpty<BUSER_WIDTH>::T BUser;
  typedef typename nvhls::UIntOrEmpty<RUSER_WIDTH>::T RUser;
  
  // ACE extensions on AW - AR - R
  typedef typename nvhls::UIntOrEmpty<C_SNOOP_WIDTH>::T   Snoop;
  typedef typename nvhls::UIntOrEmpty<C_DOMAIN_WIDTH>::T  Domain;
  typedef typename nvhls::UIntOrEmpty<C_BARRIER_WIDTH>::T Barrier;
  
  typedef typename nvhls::UIntOrEmpty<C_UNIQUE_WIDTH>::T   Unique;
  

  /**
   * \brief A struct composed of the signals associated with AXI read and write requests.
   */
  struct AddrPayload : public nvhls_message {
    Id id;
    Addr addr;
    Burst burst;
    BeatNum len;    // A*LEN
    BeatSize size;  // A*SIZE
    Cache cache;
    AUser auser;
    
    //ACE extension 
    Snoop   snoop;
    Domain  domain;
    Barrier barrier;
    
    Unique unique; // Only Used by AW. Consider split the AddrPayload into separate classes

    static const unsigned int width = ADDR_WIDTH + ID_WIDTH + ALEN_WIDTH +
                                      ASIZE_WIDTH + BURST_WIDTH + CACHE_WIDTH +
                                      AUSER_WIDTH +
                                      C_SNOOP_WIDTH + C_DOMAIN_WIDTH + C_BARRIER_WIDTH + C_UNIQUE_WIDTH;

   AddrPayload() {
     if(ID_WIDTH > 0)
       id = 0;
     addr = 0; // NVUINT, ADDR_WIDTH always > 0
     if(ALEN_WIDTH > 0)
       len = 0;
     if(ASIZE_WIDTH > 0)
       size = 0;
     if(BURST_WIDTH > 0)
       burst = 0;
     if(CACHE_WIDTH > 0)
       cache = 0;
     if(AUSER_WIDTH > 0)
       auser = 0;
     if(C_SNOOP_WIDTH > 0)
       snoop = 0;
     if(C_DOMAIN_WIDTH > 0)
       domain = 0;
     if(C_BARRIER_WIDTH > 0)
       barrier = 0;
     if(C_UNIQUE_WIDTH > 0)
       unique = 0;
    }
    
    template <unsigned int Size>
    void Marshall(Marshaller<Size> &m) {
      m &id;
      m &addr;
      m &len;
      m &size;
      m &burst;
      m &cache;
      m &auser;
      m &snoop;
      m &domain;
      m &barrier;
      m &unique;
    }

#ifdef CONNECTIONS_SIM_ONLY
    inline friend void sc_trace(sc_trace_file *tf, const AddrPayload& v, const std::string& NAME ) {
      sc_trace(tf,v.id,    NAME + ".id");
      sc_trace(tf,v.addr,  NAME + ".addr");
      sc_trace(tf,v.len,   NAME + ".len");
      if (Cfg::useACE) {
        sc_trace(tf,v.snoop,    NAME + ".snoop");
        sc_trace(tf,v.domain,  NAME + ".domain");
        sc_trace(tf,v.barrier,   NAME + ".barrier");
        sc_trace(tf,v.unique,   NAME + ".unique");
      }
    }

    inline friend std::ostream& operator<<(ostream& os, const AddrPayload& rhs)
    {
#ifdef LOG_MSG_WIDTHS
      os << std::dec;
      os << "id:" << rhs.id.width << " ";
      os << "addr:" << rhs.addr.width << " ";
      os << "len:" << rhs.len.width << " ";
      os << "size:" << rhs.size.width << " ";
      os << "burst:" << rhs.burst.width << " ";
      os << "cache:" << rhs.cache.width << " ";
      os << "auser:" << rhs.auser.width << " ";
      if (Cfg::useACE) {
        os << "snoop:" << rhs.snoop.width << " ";
        os << "domain:" << rhs.domain.width << " ";
        os << "barrier:" << rhs.barrier.width << " ";
        os << "unique:" << rhs.unique.width << " ";
      }
#else
      os << std::hex;
      os << "Id:" << rhs.id << " ";
      os << "Addr:" << rhs.addr << " ";
      os << "Len:" << rhs.len << " ";
      os << "Sz:" << rhs.size << " ";
      os << "Bu:" << rhs.burst << " ";
      if (CACHE_WIDTH)
        os << "csh:" << rhs.cache << " ";
      if (AUSER_WIDTH)
        os << "Us:" << rhs.auser << " ";
      if (Cfg::useACE) {
        os << "--ACE-- ";
        os << "Snp:" << rhs.snoop << " ";
        os << "Dom:" << rhs.domain << " ";
        os << "Bar:" << rhs.barrier << " ";
        os << "Unq:" << rhs.unique << " ";
      }
      os << std::dec;
#endif
      return os;
    }

#endif
  };

  /**
   * \brief A struct composed of the signals associated with an AXI read
   * response.
   */
  struct ReadPayload : public nvhls_message {
    Id id;
    Data data;
    Resp resp;
    Last last;
    RUser ruser;

    static const unsigned int width =
        DATA_WIDTH + RESP_WIDTH + ID_WIDTH + LAST_WIDTH + RUSER_WIDTH;

   ReadPayload() {
     if(ID_WIDTH > 0)
       id = 0;
     data = 0; // NVUINT, DATA_WIDTH always > 0 
     resp = 0; // NVUINT, RESP_WIDTH always > 0
     if(LAST_WIDTH > 0)
       last = 0;
     if(RUSER_WIDTH > 0)
       ruser = 0;
   }
    
    template <unsigned int Size>
    void Marshall(Marshaller<Size> &m) {
      m &id;
      m &data;
      m &resp;
      m &last;
      m &ruser;
    }

#ifdef CONNECTIONS_SIM_ONLY
    inline friend void sc_trace(sc_trace_file *tf, const ReadPayload& v, const std::string& NAME ) {
      sc_trace(tf,v.id,    NAME + ".id");
      sc_trace(tf,v.data,  NAME + ".data");
      sc_trace(tf,v.last,  NAME + ".last");
    }

    inline friend std::ostream& operator<<(ostream& os, const ReadPayload& rhs)
    {
#ifdef LOG_MSG_WIDTHS
      os << std::dec;
      os << "id:" << rhs.id.width << " ";
      os << "data:" << rhs.data.width << " ";
      os << "resp:" << rhs.resp.width << " ";
      os << "last:" << rhs.last.width << " ";
      os << "ruser:" << rhs.ruser.width << " ";
#else
      os << std::hex;
      os << "Id:" << rhs.id << " ";
      os << "Data:" << rhs.data << " ";
      os << "Resp:" << rhs.resp << " ";
      os << "Last:" << rhs.last << " ";
      os << "Usr:" << rhs.ruser << " ";
      os << std::dec;
#endif
      return os;
    }
#endif
  };

  /**
   * \brief A struct composed of the signals associated with an AXI write
   * response.
   */
  struct WRespPayload : public nvhls_message {
    BId id;
    Resp resp;
    BUser buser;

    static const unsigned int width = RESP_WIDTH + BID_WIDTH + BUSER_WIDTH;

   WRespPayload() {
     if(ID_WIDTH > 0)
       id = 0;
     resp = 0; // NVUINT, RESP_WIDTH always > 0
     if(BUSER_WIDTH > 0)
       buser = 0;
    }

    template <unsigned int Size>
    void Marshall(Marshaller<Size> &m) {
      m &id;
      m &resp;
      m &buser;
    }

#ifdef CONNECTIONS_SIM_ONLY
    inline friend void sc_trace(sc_trace_file *tf, const WRespPayload& v, const std::string& NAME ) {
      sc_trace(tf,v.id,    NAME + ".id");
      sc_trace(tf,v.resp,  NAME + ".resp");
    }

    inline friend std::ostream& operator<<(ostream& os, const WRespPayload& rhs)
    {
#ifdef LOG_MSG_WIDTHS
      os << std::dec;
      os << "id:" << rhs.id.width << " ";
      os << "resp:" << rhs.resp.width << " ";
      os << "buser:" << rhs.buser.width << " ";
#else
      os << std::hex;
      os << "Id:"   << rhs.id << " ";
      os << "Resp:" << rhs.resp << " ";
      os << "Usr:"  << rhs.buser << " ";
#endif
      return os;
    }
#endif
  };

  /**
   * \brief A struct composed of the signals associated with AXI write data.
   */
  struct WritePayload : public nvhls_message {
    // no id here!
    Data data;
    Last last;
    Wstrb wstrb;
    WUser wuser;

    WritePayload() {
     data = 0; // NVUINT, DATA_WIDTH always > 0 
     if(LAST_WIDTH > 0)
       last = 0;
     if(WSTRB_WIDTH > 0)
        wstrb = ~0;
     if(WUSER_WIDTH > 0)
       wuser = 0;
    }
    
    static const unsigned int width =
        DATA_WIDTH + LAST_WIDTH + WSTRB_WIDTH + WUSER_WIDTH;

    template <unsigned int Size>
    void Marshall(Marshaller<Size> &m) {
      m &data;
      m &last;
      m &wstrb;
      m &wuser;
    }

#ifdef CONNECTIONS_SIM_ONLY
    inline friend void sc_trace(sc_trace_file *tf, const WritePayload& v, const std::string& NAME ) {
      sc_trace(tf,v.data,  NAME + ".data");
      sc_trace(tf,v.last,  NAME + ".last");
      sc_trace(tf,v.wstrb, NAME + ".wstrb");
    }

    inline friend std::ostream& operator<<(ostream& os, const WritePayload& rhs)
    {
#ifdef LOG_MSG_WIDTHS
      os << std::dec;
      os << "data:" << rhs.data.width << " ";
      os << "last:" << rhs.last.width << " ";
      os << "wstrb:" << rhs.wstrb.width << " ";
      os << "wuser:" << rhs.wuser.width << " ";
#else
      os << std::hex;
      os << "Data:" << rhs.data << " ";
      os << "Last:" << rhs.last << " ";
      os << "Strb:" << rhs.wstrb << " ";
      os << "Usr:" << rhs.wuser << " ";
      os << std::dec;
#endif
      return os;
    }
#endif
  };

  /**
   * \brief The AXI read class.
   *
   * Each Connections implementation contains two ready-valid interfaces, AR for
   * read requests and R for read responses.
   */
  class read {
   public:
    /**
     * \brief The AXI read channel, used for connecting an AXI master and AXI slave.
     */
    template <Connections::connections_port_t PortType = AUTO_PORT>
    class chan {
     public:
     typedef Connections::Combinational<AddrPayload, PortType> ARChan;
     typedef Connections::Combinational<ReadPayload, PortType> RChan;

      ARChan ar;  // master to slave
      RChan r;    // slave to master

      chan(const char *name)
          : ar(nvhls_concat(name, "_ar")), r(nvhls_concat(name, "_r")){};

      // TODO: Implement AXI protocol checker
    }; // read::chan

    /**
     * \brief The AXI read master port.  This port has an AR request channel as output and an R response channel as input.
     */
    template <Connections::connections_port_t PortType = AUTO_PORT>
    class master {
     public:
      typedef Connections::Out<AddrPayload, PortType> ARPort;
      typedef Connections::In<ReadPayload, PortType> RPort;

      ARPort ar;
      RPort r;

      master(const char *name)
          : ar(nvhls_concat(name, "_ar")), r(nvhls_concat(name, "_r")) {}

      void reset() {
        ar.Reset();
        r.Reset();
      }

      ReadPayload query(const AddrPayload &addr) {
        // TODO: add nb version with state
        ar.Push(addr);
        return r.Pop();
      }

      template <class C>
      void operator()(C &c) {
        ar(c.ar);
        r(c.r);
      }
    }; // read::master

    /**
     * \brief The AXI read slave port.  This port has an AR request channel as input and an R response channel as output.
     */
    template <Connections::connections_port_t PortType = AUTO_PORT>
    class slave {
     public:
      typedef Connections::In<AddrPayload, PortType> ARPort;
      typedef Connections::Out<ReadPayload, PortType> RPort;

      ARPort ar;
      RPort r;

      slave(const char *name)
          : ar(nvhls_concat(name, "_ar")), r(nvhls_concat(name, "_r")) {}

      void reset() {
        ar.Reset();
        r.Reset();
      }

      AddrPayload aread() { return ar.Pop(); }

      bool nb_aread(AddrPayload &addr) { return ar.PopNB(addr); }

      void rwrite(const ReadPayload &data) { r.Push(data); }

      bool nb_rwrite(const ReadPayload &data) { return r.PushNB(data); }

      template <class C>
      void operator()(C &c) {
        ar(c.ar);
        r(c.r);
      }
    }; // read::slave
  }; // read

  /**
   * \brief The AXI write class.
   *
   * Each Connections implementation contains three ready-valid interfaces: AW
   * for write requests, W for write data, and B for write responses.
   */
  class write {
   public:
    /**
     * \brief The AXI write channel, used for connecting an AXI master and AXI slave.
     */
    template <Connections::connections_port_t PortType = AUTO_PORT>
    class chan {
     public:
     typedef Connections::Combinational<AddrPayload, PortType> AWChan;
     typedef Connections::Combinational<WritePayload, PortType> WChan;
     typedef Connections::Combinational<WRespPayload, PortType> BChan;

      AWChan aw;  // master to slave
      WChan w;    // master to slave
      BChan b;    // slave to master

      chan(const char *name)
          : aw(nvhls_concat(name, "_aw")),
            w(nvhls_concat(name, "_w")),
            b(nvhls_concat(name, "_b")){};

      // TODO: Implement AXI protocol checker
    };  // write::chan

    /**
     * \brief The AXI write master port.  This port has AW and W request channels as outputs and a B response channel as input.
     */
    template <Connections::connections_port_t PortType = AUTO_PORT>
    class master {
     public:
      typedef Connections::Out<AddrPayload, PortType> AWPort;
      typedef Connections::Out<WritePayload, PortType> WPort;
      typedef Connections::In<WRespPayload, PortType> BPort;

      AWPort aw;
      WPort w;
      BPort b;

      master(const char *name)
          : aw(nvhls_concat(name, "_aw")),
            w(nvhls_concat(name, "_w")),
            b(nvhls_concat(name, "_b")) {}

      void reset() {
        aw.Reset();
        w.Reset();
        b.Reset();
      }

      WRespPayload write(const AddrPayload &addr, const WritePayload &data) {
        // TODO: add nb version with state
        aw.Push(addr);
        w.Push(data);
        return b.Pop();
      }

      template <class C>
      void operator()(C &c) {
        aw(c.aw);
        w(c.w);
        b(c.b);
      }
    };  // write::master

    /**
     * \brief The AXI write slave port.  This port has AW and W request channels as inputs and a B response channel as output.
     */
    template <Connections::connections_port_t PortType = AUTO_PORT>
    class slave {
     public:
      typedef Connections::In<AddrPayload, PortType> AWPort;
      typedef Connections::In<WritePayload, PortType> WPort;
      typedef Connections::Out<WRespPayload, PortType> BPort;

      AWPort aw;
      WPort w;
      BPort b;

      bool got_waddr;
      AddrPayload stored_waddr;

      slave(const char *name)
          : aw(nvhls_concat(name, "_aw")),
            w(nvhls_concat(name, "_w")),
            b(nvhls_concat(name, "_b")),
            got_waddr(false) {}

      void reset() {
        aw.Reset();
        w.Reset();
        b.Reset();
      }

      void wread(AddrPayload &addr, WritePayload &data) {
        addr = aw.Pop();
        data = w.Pop();
      }

      bool nb_wread(AddrPayload &addr, WritePayload &data) {
        if (!got_waddr) {
          if (!aw.PopNB(addr)) {
            return false;
          } else {
            got_waddr = true;
            stored_waddr = addr;
          }
        } else {
          addr = stored_waddr;
        }

        if (w.PopNB(data)) {
          got_waddr = false;
          return true;
        } else {
          return false;
        }
      }

      void bwrite(const WRespPayload &resp) { b.Push(resp); }

      bool nb_bwrite(const WRespPayload &resp) { return b.PushNB(resp); }

      template <class C>
      void operator()(C &c) {
        aw(c.aw);
        w(c.w);
        b(c.b);
      }
    }; // write::slave
  }; // write
}; // axi_for_ace
}; // ace

#endif // _AXI_FOR_ACE_H_
