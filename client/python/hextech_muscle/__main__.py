import argparse

from .hextech import HexTechMuscle


class HexTechShell:
    def __init__(self, board: HexTechMuscle) -> None:
        super().__init__()

        self.board = board
        print("HexTech Interactive Command Prompt")
        print(f"Using device: {self.board.device}")

    def run(self) -> None:
        while True:
            try:
                user_input = input(">>> ")
                user_input = user_input.lower().strip()

                if user_input == "exit":
                    return
                elif user_input == "":
                    print(self.board.conn.readline().decode())
                else:
                    self.board._send(user_input)
            except (KeyboardInterrupt, EOFError):
                return
            except Exception as e:
                print(f"Error: {type(e)} {e}")


def run():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--device')
    parser.add_argument('-c', '--command')
    args = parser.parse_args()

    board = HexTechMuscle(args.device)

    if args.command:
        board._send(args.command)
    else:
        HexTechShell(board).run()


if __name__ == "__main__":
    run()
