#include "mem/port.hh"
#include "params/SimpleMemObject.hh"
#include "sim/sim_object.hh"

namespace gem5
{

    class SimpleMemObject: public SimObject
    {
        private:
            class CPUSidePort: public ResponsePort {
                private:
                    SimpleMemObject *owner;
                    PacketPtr blockedPacket;
                    bool needRetry; 
                public:
                    CPUSidePort(const std::string& name, SimpleMemObject *owner):
                        SlavePort(name, owner), owner(owner),
                        blockedPacket(nullptr), needRetry(false)
                    {}

                    AddrRangeList getAddrRanges() const override;
                protected:
                    Tick recvAtomic(PacketPtr pkt) override { panic("recvAtomic unimpl."); }
                    void recvFunctional(PacketPtr pkt) override;
                    bool recvTimingReq(PacketPtr pkt) override;
                    void recvRespRetry() override;
            };

            class MemSidePort: public RequestPort {
                private:
                    SimpleMemObject *owner;
                    PacketPtr blockedPacket;
                public:
                    MemSidePort(const std::string& name, SimpleMemObject *owner):
                        MasterPort(name, owner), owner(owner),
                        blockedPacket(nullptr)
                    {}
                protected:
                    bool recvTimingResp(PacketPtr pkt) override;
                    void recvReqRetry() override;
                    void recvRangeChange() override;
            };

            CPUSidePort instPort;
            CPUSidePort dataPort;

            MemSidePort memPort;

            bool blocked;
        public:
            
            SimpleMemObject(const SimpleMemObjectParams &params);

            Port& getPort(const std::string &if_name, PortID idx = InvalidPortID) override;
    };

}