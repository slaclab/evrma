### This sets up the environment to test two VEVRs on the EVR230 card.

cleanup.sh
setup.sh
vevr-evr230.sh
evrManager /dev/evr0mng destroy vevr1 >/dev/null
evrManager /dev/evr0mng create vevr1
sleep 1
chmod 666 /dev/vevr1


