
# DEST="matter@192.168.0.128"
DEST="matter@192.168.0.217"
ssh $DEST mkdir -p /home/matter/WebUI
scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ./index.html $DEST:/home/matter/WebUI
scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null ./webUI.service $DEST:/home/matter/WebUI
ssh $DEST sudo systemctl stop webUI.service 
ssh $DEST sudo cp /home/matter/WebUI/webUI.service /lib/systemd/system
ssh $DEST sudo systemctl enable webUI.service 
ssh $DEST sudo systemctl start webUI.service 