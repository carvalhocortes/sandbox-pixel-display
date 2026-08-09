#!/usr/bin/env python3
"""Send the computer's local time to the display firmware over serial."""

import argparse
from datetime import datetime
import sys
import time


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="porta serial do NodeMCU")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    try:
        import serial
    except ImportError:
        print("Dependencia ausente: instale pyserial para usar este comando.", file=sys.stderr)
        return 1

    computer_time = datetime.now().replace(microsecond=0)
    command = f"SYNC {computer_time:%Y-%m-%d %H:%M:%S}\n"

    try:
        with serial.Serial(args.port, args.baud, timeout=1) as connection:
            time.sleep(2)
            connection.write(command.encode("ascii"))
            connection.flush()

            print(f"Enviado: {command.strip()}")
            deadline = time.monotonic() + 12
            while time.monotonic() < deadline:
                response = connection.readline().decode("utf-8", errors="replace").strip()
                if response:
                    print(response)
                    if response.startswith("RTC ajustado para:"):
                        return 0
    except serial.SerialException as error:
        print(f"Nao foi possivel acessar {args.port}: {error}", file=sys.stderr)
        return 1

    print("O firmware nao confirmou o ajuste do RTC.", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
