import asyncio
import websockets
import json
import time

WALL_DIST = 20
TIME_TURN = 1
last_turn = 'блять..'
forced_turns = False


async def send_command(ws, command):
    try:
        await ws.send(json.dumps({"direction": command}))
    except websockets.ConnectionClosed:
        print("Connection closed while sending command.")

async def small_move(ws, direction):
    start_time = time.time()
    while time.time() - start_time < TIME_TURN:
            await send_command(ws, direction)
            await asyncio.sleep(0.1) #TO DO

async def turn(ws, direction):
    if forced_turns:
        forced_turns = False

    small_move(ws, direction)
    small_move(ws, 'forward')
    
    message = await ws.recv()
    data = json.loads(message)
    distance = data.get("distance", 1000)

    if distance < WALL_DIST:
        forced_turns = True
    else:
        last_turn = direction
    
    small_move(ws, direction)


async def main():
    global last_turn, forced_turns
    uri = "ws://localhost:8765"

    async with websockets.connect(uri) as ws:
        while True:
            try:
                message = await ws.recv()
                data = json.loads(message)
                distance = data.get("distance", 1000)

                if distance < WALL_DIST:
                    if last_turn == 'right':
                        turn(ws, 'left')
                    elif last_turn == 'left':
                        turn(ws, 'right')

            except websockets.ConnectionClosed:
                print("Connection closed.")
                break
