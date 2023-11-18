import os

from fastapi import BackgroundTasks, FastAPI
import tinytuya


local_key = os.environ["TUYA_LOCAL_KEY"]
d = tinytuya.OutletDevice(
    dev_id="eb48d3aaf9a6bae052ceiy",
    address="172.16.1.4",
    local_key=local_key,
    version=3.4,
)
d.set_socketPersistent(True)
print(" > Send Request for Status < ")
payload = d.generate_payload(tinytuya.DP_QUERY)
d.send(payload)
data = {}


async def fetch_data():
    global data

    received = d.receive()
    if received is None:
        return
    if "Err" in received:
        print(f"error: {received}")
        return
    if "dps" in received:
        data["dps"] = data.get("dps", {}) | received["dps"]
    if "t" in received:
        data["t"] = received["t"]
    print(data)


app = FastAPI()


@app.get("/")
async def status(background_tasks: BackgroundTasks):
    background_tasks.add_task(fetch_data)
    return data
