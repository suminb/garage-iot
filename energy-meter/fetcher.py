import os

import tinytuya


local_key = os.environ["TUYA_LOCAL_KEY"]
d = tinytuya.OutletDevice(
    dev_id="eb48d3aaf9a6bae052ceiy",
    address="172.16.1.39",
    local_key=local_key,
    version=3.4,
)
d.set_socketPersistent(True)

print(" > Send Request for Status < ")
payload = d.generate_payload(tinytuya.DP_QUERY)
d.send(payload)

for _ in range(100):
    print(d.receive())

    # Send keyalive heartbeat
    payload = d.generate_payload(tinytuya.HEART_BEAT)
    d.send(payload)