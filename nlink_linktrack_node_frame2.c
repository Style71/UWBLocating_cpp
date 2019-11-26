﻿#include "nlink_linktrack_node_frame2.h"

NodeFrame2Data nodeFrame2Data_={0};

void unpackNodeFrame2Data(uint8_t *byteArray)
{
    const int framePartLength = 119;
    static uint8_t initNeeded = 1;
    if (initNeeded)
    {
        memset(nodeFrame2Data_.node, 0, sizeof(nodeFrame2Data_.node));
        initNeeded = 0;
        assert(sizeof(nodeFrame2Data_.framePart) == framePartLength);
    }

    memcpy(&nodeFrame2Data_.framePart, byteArray, framePartLength);

    for (int i = 0; i < nodeFrame2Data_.framePart.validNodeQuantity; ++i)
    {
        RawNode2_t rawNode;
        memcpy(&rawNode, byteArray + framePartLength + i * sizeof(RawNode2_t), sizeof(RawNode2_t));

        if (!nodeFrame2Data_.node[rawNode.id])
        {
            nodeFrame2Data_.node[rawNode.id] = malloc(sizeof(Node2_t));
        }
        nodeFrame2Data_.node[rawNode.id]->role = rawNode.role;
        nodeFrame2Data_.node[rawNode.id]->id = rawNode.id;
        nodeFrame2Data_.node[rawNode.id]->dis = int24Value(rawNode.disTemp) / kDisMultiply_;
        nodeFrame2Data_.node[rawNode.id]->fpRssi = rawNode.fpRssiTemp / kRssiMultiply_;
        nodeFrame2Data_.node[rawNode.id]->rxRssi = rawNode.rxRssiTemp / kRssiMultiply_;
        nodeFrame2Data_.node[rawNode.id]->systemTime = rawNode.systemTime;

        nodeFrame2Data_.nodeIDList[i]=rawNode.id;
    }

    int24ToFloat(nodeFrame2Data_.framePart.posTemp, nodeFrame2Data_.pos,
                 sizeof(nodeFrame2Data_.pos) / sizeof(nodeFrame2Data_.pos[0]),
                 kPosMultiply_);

    int24ToFloat(nodeFrame2Data_.framePart.velTemp, nodeFrame2Data_.vel,
                 sizeof(nodeFrame2Data_.vel) / sizeof(nodeFrame2Data_.vel[0]),
                 kVelMultiply_);

    int16ToFloat(nodeFrame2Data_.framePart.angleTemp, nodeFrame2Data_.angle,
                 sizeof(nodeFrame2Data_.angle) / sizeof(nodeFrame2Data_.angle[0]),
                 kAngleMultiply_);
    uint8ToFloat(nodeFrame2Data_.framePart.eopTemp, nodeFrame2Data_.eop,
                 sizeof(nodeFrame2Data_.eop) / sizeof(nodeFrame2Data_.eop[0]),
                 kEopMultiply_);

    nodeFrame2Data_.supplyVoltage = nodeFrame2Data_.framePart.supplyVoltageTemp / kVoltageMultiply_;

    //    NodeFrame2Data data = nodeFrame2Data_;
}

uint8_t verifyNodeFrame2Data(uint8_t *byteArray, int32_t length)
{
    if (byteArray[0] != 0x55 || byteArray[1] != 0x04)
        return 0;
    return verifyCheckSum(byteArray, length);
}
