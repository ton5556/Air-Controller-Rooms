Protocol=PulseDistance Raw-Data=0x6000000 64 bits LSB first Gap=3276750us Duration=67450us
Send on a 8 bit platform with: 
    uint32_t tRawData[]={0x4E02002, 0x6000000};
    IrSender.sendPulseDistanceWidthFromArray(38, 3500, 1650, 450, 1300, 450, 450, &tRawData[0], 64, PROTOCOL_IS_LSB_FIRST, <RepeatPeriodMillis>, <numberOfRepeats>);

Protocol=PulseDistance Raw-Data=0x6000000 64 bits LSB first Gap=3276750us Duration=67450us
Send on a 8 bit platform with: 
    uint32_t tRawData[]={0x4E02002, 0x6000000};
    IrSender.sendPulseDistanceWidthFromArray(38, 3450, 1700, 450, 1300, 450, 400, &tRawData[0], 64, PROTOCOL_IS_LSB_FIRST, <RepeatPeriodMillis>, <numberOfRepeats>);
