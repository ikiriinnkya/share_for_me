# -*- coding: utf-8 -*-
import cv2
import numpy as np
import time

color = {
    "gray":(255,0,0),
}

def red_detect(img):
    # HSV�F���Ԃɕϊ�
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)#�啶���̓t���b�O�����݂�

    # �ԐF��HSV�̒l��1
    hsv_min = np.array([0,127,0])
    hsv_max = np.array([20,255,255])
    mask1 = cv2.inRange(hsv, hsv_min, hsv_max)#�Ԃ����擾

    # �ԐF��HSV�̒l��2
    hsv_min = np.array([160,127,0])
    hsv_max = np.array([179,255,255])
    mask2 = cv2.inRange(hsv, hsv_min, hsv_max)

    return mask1 + mask2


def main():
    # �J�����̃L���v�`��
    cap = cv2.VideoCapture(0)

    while(cap.isOpened()):
        # �t���[�����擾
        ret, frame = cap.read()

        # �ԐF���o
        mask = red_detect(frame)

        # ���ʕ\��
        #cv2.imshow("Frame", frame)
        #cv2.imshow("Mask", mask)
        m = cv2.moments(mask)
        cx = int(m["m10"] // m["m00"])
        cy = int(m["m01"] // m["m00"])
        cv2.drawMarker(mask, (cx,cy), color["gray"])
        cv2.imshow("img",mask)
        print("x",cx)
        print("y",cy)
        time.sleep(1)
        # q�L�[�������ꂽ���r���I��
        if cv2.waitKey(25) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
