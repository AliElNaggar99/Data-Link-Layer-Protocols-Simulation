enum M_Type{
    ACK,  // we send data with ack each time so no need for this one anymore
    NACK, // we made it separateType as we send nack without data
    DATA, // we send data with ack each time
    Self_Message,
    TimeOut,
    Duplicate,
    ACK_TimeOut, // this happens when we dont have data , so we send ack alone
    NACK_TimeOut
};
