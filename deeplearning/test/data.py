import sys
import random
import argparse
import math


parser = argparse.ArgumentParser(description='WIFI RSSI 기반으로 데이터를 학습할 예제입니다')

parser.add_argument('N', metavar='N', type=int, help='생성할 데이터의 갯수입니다')
parser.add_argument('--width', metavar='width', type=int, default=1000, help='맵 너비 입니다')
parser.add_argument('--height', metavar='height', type=int, default=1000, help='맵 높이 입니다')
parser.add_argument('--router', metavar='router', type=int, default=5, help='WIFI 장치 갯수입니다')
parser.add_argument('--noise', metavar='noise', type=float, default=0.0, help='노이즈 정도입니다')

args = parser.parse_args()

router = []
pos = []
x = []
y = []

router = []

for i in range(args.router):
    router.append([random.randint(0, args.width), random.randint(0, args.height)])

for i in range(args.N):
    p = [random.randint(0, args.width), random.randint(0, args.height)]
    x.append(p)
    rssi = []
    for r in router:
        rssi.append(math.sqrt(pow(p[0] - r[0], 2) + pow(p[1] - r[1], 2)) + random.random() * args.noise)
    y.append(rssi)

print(x)
print(y)
