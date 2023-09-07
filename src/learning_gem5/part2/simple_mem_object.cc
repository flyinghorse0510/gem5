#include "learning_gem5/part2/simple_mem_object.hh"
#include "debug/SimpleMemObject.hh"

namespace gem5
{
    SimpleMemObject::SimpleMemObject(const SimpleMemObjectParams& params):
        SimObject(params),
        instPort(params.name + ".inst_port", this),
        dataPort(params.name + ".data_port", this),
        memPort(params.name + ".mem_port", this),
        blocked(false)
    {}

    Port&
    SimpleMemObject::getPort(const std::string &if_name, PortID idx = InvalidPortID)
    {
        panic_if(idx != InvalidPortID, "This object doesn't support vector ports");

        if (if_name == "mem_side") {
            return memPort;
        } else if (if_name == "inst_port") {
            return instPort;
        } else if (if_name == "data_port") {
            return dataPort;
        } else {
            return SimObject::getPort(if_name, idx);
        }
    }

    AddrRangeList
    SimpleMemObject::CPUSidePort::getAddrRanges() const {
        return owner->getAddrRanges();
    }

    void
    SimpleMemObject::CPUSidePort::recvFunctional(PacketPtr pkt) {
        return owner->handleFunctional(pkt);
    }

    AddrRangeList
    SimpleMemObject::getAddrRanges() const {
        DPRINTF(SimpleMemObject, "Sending new ranges\n");
        return memPort.getAddrRanges();
    }

    void
    SimpleMemObject::handleFunctional(PacketPtr pkt) {
        memPort.sendFunctional(pkt);
    }

    void
    SimpleMemObject::MemSidePort::recvRangeChange() {
        owner->sendRangeChange();
    }

    void
    SimpleMemObject::sendRangeChange() {
        instPort.sendRangeChange();
        dataPort.sendRangeChange();
    }

    bool
    SimpleMemObject::CPUSidePort::recvTimingReq(PacketPtr pkt) {
        if (!owner->handleRequest(pkt)) {
            needRetry = true;
            return false;
        } else {
            return true;
        }
    }

    bool 
    SimpleMemObject::handleRequest(PacketPtr pkt) {
        if (blocked) {
            return false;
        }

        DPRINTF(SimpleMemObject, "Got request for addr %#x\n", pkt->getAddr());
        blocked = true;
        memPort.sendPacket(pkt);
        return true;
    }

    void 
    SimpleMemObject::MemSidePort::sendPacket(PacketPtr pkt) {
        panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");
        if (!sendTimingReq(pkt)) {
            blockedPacket = pkt;
        }
    }

    void
    SimpleMemObject::MemSidePort::recvReqRetry() {
        assert(blockedPacket != nullptr);

        PacketPtr pkt = blockedPacket;
        blockedPacket = nullptr;

        sendPacket(pkt);
    }

    bool
    SimpleMemObject::MemSidePort::recvTimingResp(PacketPtr pkt) {
        return owner->handleResponse(pkt);
    }


    bool
    SimpleMemObject::handleResponse(PacketPtr pkt) {
        assert(blocked);
        DPRINTF(SimpleMemObject, "Got response for addr %#x\n", pkt->getAddr());

        blocked = false;
        if (pkt->req->isInstFetch()) {
            instPort.sendPacket(pkt);
        } else {
            dataPort.sendPacket(pkt);
        }

        instPort.trySendRetry();
        dataPort.trySendRetry();

        return true;
    }

    void
    SimpleMemObject::CPUSidePort::sendPacket(PacketPtr pkt) {
        panic_if(blockedPacket != nullptr, "Should never try to send if blocked!");

        if (!sendTimingResp(pkt)) {
            blockedPacket = pkt;
        }
    }

    void
    SimpleMemObject::CPUSidePort::recvRespRetry() {
        assert(blockedPacket != nullptr);

        PacketPtr pkt = blockedPacket;
        blockedPacket = nullptr;

        sendPacket(pkt);
    };

    void
    SimpleMemObject::CPUSidePort::trySendRetry() {
        if (needRetry && blockedPacket == nullptr) {
            needRetry = false;
            DPRINTF(SimpleMemObject, "Sending retry req for %d\n", id);
            sendRetryReq();
        }
    }

    // SimpleMemObject*
    // SimpleMemobjParams::create() {
    //     return new SimpleMemobj(this);
    // }


}