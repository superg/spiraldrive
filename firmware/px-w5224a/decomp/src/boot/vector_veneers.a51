; Fixed four-byte reset/interrupt entry veneers for the main firmware image.
; The C251 interrupt bodies retain compiler-generated save/restore and RETI.
$CASE
NAME VECTOR_VENEERS

PUBLIC ResetVector?
PUBLIC ControllerEventInterruptEntry?
PUBLIC HostInterfaceInterruptEntry?
PUBLIC ControllerPacketInterruptEntry?
PUBLIC Timer0InterruptEntry?
PUBLIC SfrE1Bit7InterruptEntry?

EXTRN ECODE (FirmwareMainLoop?)
EXTRN ECODE (ControllerEventInterruptBody?)
EXTRN ECODE (HostInterfaceInterruptBody?)
EXTRN ECODE (ControllerPacketInterruptBody?)
EXTRN ECODE (Timer0InterruptBody?)
EXTRN ECODE (SfrE1Bit7InterruptBody?)

RESET_VECTOR_VENEER SEGMENT ECODE
RSEG RESET_VECTOR_VENEER
ResetVector?:
    EJMP FirmwareMainLoop?

CONTROLLER_EVENT_VECTOR_VENEER SEGMENT ECODE
RSEG CONTROLLER_EVENT_VECTOR_VENEER
ControllerEventInterruptEntry?:
    EJMP ControllerEventInterruptBody?

HOST_INTERFACE_VECTOR_VENEER SEGMENT ECODE
RSEG HOST_INTERFACE_VECTOR_VENEER
HostInterfaceInterruptEntry?:
    EJMP HostInterfaceInterruptBody?

CONTROLLER_PACKET_VECTOR_VENEER SEGMENT ECODE
RSEG CONTROLLER_PACKET_VECTOR_VENEER
ControllerPacketInterruptEntry?:
    EJMP ControllerPacketInterruptBody?

TIMER0_VECTOR_VENEER SEGMENT ECODE
RSEG TIMER0_VECTOR_VENEER
Timer0InterruptEntry?:
    EJMP Timer0InterruptBody?

SFR_E1_BIT7_VECTOR_VENEER SEGMENT ECODE
RSEG SFR_E1_BIT7_VECTOR_VENEER
SfrE1Bit7InterruptEntry?:
    EJMP SfrE1Bit7InterruptBody?

END
