
% thoth.hdr

.Send( receiverid, data )

\ try to send a message to process receiver

{
    extrn .Active, .Pid_base, .Idmask;
    auto i, receiver;
    
    MESSAGE[.Active] = data;    \ move msg now to save disable time
    
\ test for invalid receiver
    disable;
    if (ID[receiver=.Pid_base[receiverid&.Idmask]] != receiverid)
    {
        enable;
        return( ERROR );
    }   \ process does not exist
    
    if ((STATUS[receiver] == REC_BLKED) &&
       (BLOCKED_ON[receiver] == .Active))
    {

\ receiver is receive-blocked on this process
\ therefore, remove it from the receive-blocked queue
        BLOCK_BACK[BLOCK_FWD[receiver]] = BLOCK_BACK[receiver];
        BLOCK_FWD[BLOCK_BACK[receiver]] = BLOCK_FWD[receiver];
        
        MESSAGE[receiver] = data;
        
        .Add_ready(receiver);
        
        enable;
        return( OK );
    }
    
\ receive has not performed a .receive to this process yet
    BLOCKED_ON[ .Active ] = receiver;
    STATUS[ .Active ] = SEND_BLKED;
    
\ place this process in the send-blocked queue of receiver
    .Remove_ready( .Active );
    BLOCK_FWD[ .Active ] = i = SENDQ_TAIL[ receiver];
    BLOCK_BACK[ .Active ] = BLOCK_BACK[ i ];
    BLOCK_BACK[ i ] = SENDQ_TAIL[ receiver ] = .Active;
    
    if (STATUS[receiver] == WTNG_SENDER)    \ in wait-sender state
        .Add_ready(receiver);
    
    enable;
    .Dispatch();    \ block until receiver does
                    \ a .Receive to this process
    
    return( OK );
}

.Wait_sender()

\ wait until a sender is in the sender-queue

{
    extrn .Active;
    disable;
    if (SENDQ_HEAD[.Active] == &SENDQ_EMPTY[.Active])
    {
        STATUS[ .Active ] = WTNG_SENDER;
        .Remove_ready( .Active );
        .Dispatch( );
    }
    enable;
    return( ID[SENDQ_HEAD[.Active]] );
}

.Receive(senderid)

\ wait until a message is sent by the process senderid

{
    extrn .Active, .Pid_base, .Idmask;
    auto i, sender;

\ test for invalid sender
    disable;
    if (ID[sender = .Pid_base[ senderid & .Idmask]] != senderid)
    {
        BLOCKED_ON[.Active ] = sender;
        STATUS[ .Active ] = REC_BLKED;
        .Remove_ready( .Active );
        .Dispatch();    \ process is permanently blocked
    }
    if ((STATUS[sender] == SEND_BLKED) &&   \ must be send-blocked
        (BLOCKED_ON[sender] == .Active))    \ on .Active
    {
\ sender is sent-blocked on this process
\ remove him from the send-blocked queue
        BLOCK_BACK[ BLOCK_FWD[ sender ] ] = BLOCK_BACK[ sender ];
        BLOCK_FWD[ BLOCK_BACK[ sender ] ] = BLOCK_FWD[ sender ];
        
        .Add_ready(sender);
        i = MESSAGE[sender];
        enable;
        return( i );
    }
    
\ sender has not done a .send yet
    BLOCKED_ON[ .Active ] = sender;
    STATUS[ .Active ] = REC_BLKED;
    
\ put this process in the receive-blocked queue of sender
    .Remove_ready( .Active );
    BLOCK_FWD[ .Active ] = i = RECQ_TAIL[ sender ];
    BLOCK_BACK[ .Active ] = BLOCK_BACK[ i ];
    BLOCK_BACK[ i ] = RECQ_TAIL[ sender ] = .Active;
    
    if (STATUS[sender] == WTNG_RCVR)    \ in wait-receiver state
        .Add_ready(sender);
    
    enable;
\ block until sender does a .Sender to this process
    .Dispatch();
    return( MESSAGE[ .Active ] );
}

.Wait_receiver( )

\ wait until a receiver is in receive-queue

{
    extrn .Active;
    disable;
    if (RECQ_HEAD[.Active] == &RECQ_EMPTY[.Active])
    {
        STATUS[ .Active ] = WTNG_RCVR;
        .Remove_ready( .Active );
        .Dispatch( );
    }
    enable;
    return( ID[RECQ_HEAD[.Active]] );
}
