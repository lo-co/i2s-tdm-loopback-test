import pandas as pd
import os
from binascii import hexlify

# print(os.getcwd())
df = pd.read_csv('../docs/digital.csv')
# Convert time to ns
df['t'] = df['t']*1e9

bits = []
frames = []
current_frame = []
previous_bclk = 0
previous_fs = 0
current_bit = 0
fs_active = False

for i, row in df.iterrows():
    if row[2] == 1 and previous_fs == 0:
        fs_active = True
        if i != 0:
            if not frames:
                frames = [current_frame]
            else:
                frames.append(current_frame)
            current_frame = []
            bits = []
            current_bit = 0
    if previous_bclk ==0 and row[1] == 1:
        if i != 0 and not fs_active:
            current_bit += 1
        if current_bit >=8:
            current_bit = 0
            hstr = ''.join(map(str, bits))
            hval =hex(int(hstr, 2))
            # hval = hval.to_bytes((len(hstr)+7)//8, byteorder="little")
            # hval = hexlify(hval).decode()
            if current_frame:
                current_frame.append(hval)
            else:
                current_frame = [hval]
            bits = [int(row[3])]
        else:
            bits.append(int(row[3]))
        # active until next bclk
        fs_active = False
    previous_fs = row[2]
    previous_bclk = row[1]

print('all done')
