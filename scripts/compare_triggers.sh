### EVR230

evrManager /dev/evr0mng init
evrManager /dev/evr0mng destroy vevr0 >/dev/null
evrManager /dev/evr0mng create vevr0

sleep 1 ### wait for vevr to appear
chmod 666 /dev/vevr0

evrManager /dev/evr0mng alloc vevr0 output 0
evrManager /dev/evr0mng alloc vevr0 pulsegen
evrManager /dev/evr0mng output vevr0 0 P 0



### SLAC EVR

evrManager /dev/evr1mng init
evrManager /dev/evr1mng destroy vevr1 >/dev/null
evrManager /dev/evr1mng create vevr1

sleep 1 ### wait for vevr to appear
chmod 666 /dev/vevr1

evrManager /dev/evr1mng alloc vevr1 output 0
evrManager /dev/evr1mng alloc vevr1 pulsegen
evrManager /dev/evr1mng output vevr1 0 P 0



