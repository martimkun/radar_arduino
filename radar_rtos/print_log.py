import serial
import os

filepath = os.path.dirname(os.path.abspath(__file__))

def main():
    try:
        index = 0
        while(True):
            print("Tentando uart...")
            ser = serial.Serial('/dev/ttyACM2',9600)
            data = ser.read_until(size=64)

            if os.path.exists(filepath+"/log.txt"):
                file = open(filepath+"/log.txt",'a')
                index+=1

            else:
                file = open(filepath+"/log.txt",'x')

            data = data.decode('utf-8')
            print(data)

            if ("999" not in data):
                if(data[0]== 'm' and data[1]=='a'):
                    file.write(str(index)+";"+data)
            file.close()
            
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    main()
