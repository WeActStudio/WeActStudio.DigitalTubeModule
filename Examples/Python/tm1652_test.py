import serial
import sys
import time
import threading
import signal

class tm1652_test:
    def __init__(self, arg1) -> None:
        self.dig_num = [0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F]
        try:
            self.ser = serial.Serial(arg1, 19200, 8, "O", 1, timeout=0.05)
        except:
            print("open serial port fail")
            self.ser = None
            return

    def reverse4Bits(self, n):
        bits = "{:0>4b}".format(n)
        return int(bits[::-1], 2)

    def write_cmd(self, cmd):
        self.ser.write(cmd)
        time.sleep(0.006)

    def set_brightness(self, brightness):
        if brightness < 0 or brightness > 8:
            return
        else:
            send = bytearray()
            send.append(0x18)
            send.append(0x10 | ((self.reverse4Bits(brightness - 1)) & 0x0F))
            # print(send)
            self.write_cmd(send)

    def write_dig(self, dig1, dig2, dig3, dig4):
        send = bytearray()
        send.append(0x08)
        send.append(dig1)
        send.append(dig2)
        send.append(dig3)
        send.append(dig4)
        # print(send)
        self.write_cmd(send)
    
    def display_clock(self,hour,min,second):
        if second%2 == 0:
            dig2 = self.dig_num[hour%10] + 0x80
        else:
            dig2 = self.dig_num[hour%10]
        self.write_dig(self.dig_num[hour//10],dig2,self.dig_num[min//10],self.dig_num[min%10])
    
    def clear(self):
        self.write_dig(0,0,0,0)

    def close(self):
        self.clear()
        self.ser.close()

if __name__ == "__main__":
    import win32api
    t = tm1652_test(sys.argv[1])

    if t.ser == None:
        exit()
    
    def on_exit(sig, func=None):
        print('Close')
        t.close()
        sys.exit(0)
    
    def check_key():
        while True:
            if input() == 'q':
                print("quit now")
                break

    # def signal_handler(signal, frame):
    #     print('Caught Ctrl+C / SIGINT signal')
    #     t.clear()
    #     sys.exit(0)
    # signal.signal(signal.SIGINT, signal_handler)

    win32api.SetConsoleCtrlHandler(on_exit, True)

    t.clear()

    t.set_brightness(3)
    
    check_key_thread = threading.Thread(target=check_key)
    check_key_thread.start()

    print("press q and ENTER to quit.")

    tm_sec_old = 0
    while True:

        localtime = time.localtime(time.time())
        if tm_sec_old != localtime.tm_sec:
            tm_sec_old = localtime.tm_sec
            t.display_clock(localtime.tm_hour,localtime.tm_min,localtime.tm_sec)
        time.sleep(0.1)

        if check_key_thread.is_alive() == False:
            break
    print("quit")