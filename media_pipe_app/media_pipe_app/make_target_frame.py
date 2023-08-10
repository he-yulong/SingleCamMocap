# -*- coding: utf-8 -*-
"""
@File  : make_target_frame.py
@Author: Yulong He
@Date  : 2021/11/22 9:58
@Desc  : 根据用户相机，S秒之后采集相应姿态数据，使用IK转换成 VICON BVH目标帧。
"""
import time
import math
import numpy as np
import cv2
import mediapipe as mp
from retargeting.google.google2vicon_grounded_full_body import api

mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_holistic = mp.solutions.holistic
USE_HAND = True
BG_COLOR = (192, 192, 192)  # gray


def draw_landmark_annotation_on_the_image_from_cap(image, results):
    image.flags.writeable = True
    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
    mp_drawing.draw_landmarks(
        image,
        results.face_landmarks,
        mp_holistic.FACEMESH_CONTOURS,
        landmark_drawing_spec=None,
        connection_drawing_spec=mp_drawing_styles
            .get_default_face_mesh_contours_style())
    mp_drawing.draw_landmarks(
        image,
        results.pose_landmarks,
        mp_holistic.POSE_CONNECTIONS,
        landmark_drawing_spec=mp_drawing_styles
            .get_default_pose_landmarks_style())
    # Flip the image horizontally for a selfie-view display.
    cv2.imshow('MediaPipe Holistic', cv2.flip(image, 1))


def draw_landmark_annotation_on_the_image_from_static_image(image, results, idx=0):
    annotated_image = image.copy()
    # Draw segmentation on the image.
    # To improve segmentation around boundaries, consider applying a joint
    # bilateral filter to "results.segmentation_mask" with "image".
    condition = np.stack((results.segmentation_mask,) * 3, axis=-1) > 0.1
    bg_image = np.zeros(image.shape, dtype=np.uint8)
    bg_image[:] = BG_COLOR
    annotated_image = np.where(condition, annotated_image, bg_image)
    # Draw pose, left and right hands, and face landmarks on the image.
    mp_drawing.draw_landmarks(
        annotated_image,
        results.face_landmarks,
        mp_holistic.FACEMESH_TESSELATION,
        landmark_drawing_spec=None,
        connection_drawing_spec=mp_drawing_styles
            .get_default_face_mesh_tesselation_style())
    mp_drawing.draw_landmarks(
        annotated_image,
        results.pose_landmarks,
        mp_holistic.POSE_CONNECTIONS,
        landmark_drawing_spec=mp_drawing_styles.
            get_default_pose_landmarks_style())
    cv2.imwrite('assets/annotated_image' + str(idx) + '.png', annotated_image)
    # Plot pose world landmarks.
    mp_drawing.plot_landmarks(
        results.pose_world_landmarks, mp_holistic.POSE_CONNECTIONS)


def _normalized_to_pixel_coordinates(
        normalized_x, normalized_y, normalized_z, image_width,
        image_height):
    """Converts normalized value pair to pixel coordinates."""
    # Checks if the float value is between 0 and 1.
    x_px = min(math.floor(normalized_x * image_width), image_width - 1)
    y_px = min(math.floor(normalized_y * image_height), image_height - 1)
    z_px = min(math.floor(normalized_z * image_width), image_height - 1)
    return x_px, y_px, z_px


def get_landmarks(landmark_list, image_shape):
    """Plot the landmarks and the connections in matplotlib 3d.

    Args:
      landmark_list: A normalized landmark list proto message to be plotted.
      connections: A list of landmark index tuples that specifies how landmarks to
        be connected.
      landmark_drawing_spec: A DrawingSpec object that specifies the landmarks'
        drawing settings such as color and line thickness.
      connection_drawing_spec: A DrawingSpec object that specifies the
        connections' drawing settings such as color and line thickness.
      elevation: The elevation from which to view the plot.
      azimuth: the azimuth angle to rotate the plot.
    Raises:
      ValueError: If any connetions contain invalid landmark index.
    """
    if not landmark_list:
        return
    plotted_landmarks = []
    h, w = image_shape[0], image_shape[1]
    for idx, landmark in enumerate(landmark_list.landmark):
        visibility = landmark.visibility if landmark.HasField('visibility') else 1
        x_px, y_px, z_px = _normalized_to_pixel_coordinates(landmark.x, landmark.y, landmark.z, w, h)
        plotted_landmarks.append([-z_px, x_px, -y_px, visibility])
    return np.array(plotted_landmarks, dtype=np.float32)


def get_model_fingers():
    TEST_MODEL_PATH = 'E:/lightning_logs/iknet/lightning_logs/version_3/epoch34-val-normal_loss5.024345.ckpt'  # Not for API
    from iknet.model.pl_model_v1 import NaiveDNN2
    model = NaiveDNN2.load_from_checkpoint(TEST_MODEL_PATH)
    model.cuda()
    model.eval()
    return model


def extract_data_at_every_frame(image, results, use_hand=False, use_world=False):
    pose = results.pose_world_landmarks if use_world else results.pose_landmarks
    pose_landmark_px = get_landmarks(pose, image.shape[:2])  # 720, 1280, 3
    if pose_landmark_px is None:
        return None, None
    both_wrist_px = [pose_landmark_px[15][:3], pose_landmark_px[16][:3]]

    if use_hand:
        left_hand_landmark = get_landmarks(results.left_hand_landmarks, image.shape[:2])
        right_hand_landmark = get_landmarks(results.right_hand_landmarks, image.shape[:2])
        if left_hand_landmark is None:
            left_hand_landmark = np.zeros((21, 4), dtype=np.float32)
        else:
            offset_left = left_hand_landmark[0][:3] - both_wrist_px[0]
            left_hand_landmark[:, :3] -= offset_left
        if right_hand_landmark is None:
            right_hand_landmark = np.zeros((21, 4), dtype=np.float32)
        else:
            offset_right = right_hand_landmark[0][:3] - both_wrist_px[1]
            right_hand_landmark[:, :3] -= offset_right
        # if left_hand_landmark is not None:
        #     print(left_hand_landmark.shape)
        if left_hand_landmark is None:
            left_hand_landmark = np.zeros((21, 4), dtype=np.float32)
        if right_hand_landmark is None:
            right_hand_landmark = np.zeros((21, 4), dtype=np.float32)
        landmarks_pose_and_hand = np.concatenate((pose_landmark_px, left_hand_landmark, right_hand_landmark))

    return pose_landmark_px, landmarks_pose_and_hand


# This is main process function
def create_target_bvh(result_pose_and_hand, out_path, use_cuda=False):
    out_anim = api(result_pose_and_hand[np.newaxis, ...], use_cuda=use_cuda)
    out_anim = out_anim[-1:]  # 最后一帧
    out_anim.to_bvh_file().write_file(out_path)


def demo_cap():
    TIME_INTERVAL = 10
    # For webcam input:
    cap = cv2.VideoCapture(0)
    use_hand = True
    with mp_holistic.Holistic(
            min_detection_confidence=0.1,
            min_tracking_confidence=0.1) as holistic:
        t0 = time.time()
        while cap.isOpened():
            success, image = cap.read()
            if not success:
                print("Ignoring empty camera frame.")
                # If loading a video, use 'break' instead of 'continue'.
                continue
            if cv2.waitKey(5) & 0xFF == 27:
                break
            # To improve performance, optionally mark the image as not writeable to
            # pass by reference.
            image.flags.writeable = False
            image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            results = holistic.process(image)
            draw_landmark_annotation_on_the_image_from_cap(image, results)

            if time.time() - t0 > TIME_INTERVAL:
                landmarks_pose, landmarks_pose_and_hand = extract_data_at_every_frame(image, results, use_hand=use_hand,
                                                                                      use_world=False)
                if landmarks_pose is None:
                    continue
                break
    cap.release()
    create_target_bvh(landmarks_pose_and_hand, out_path='E:/save/retarget/target.bvh')


def get_target_bvh_from_static_image(IMAGE_FILES: list[str], out_path: str, use_cuda=False):
    with mp_holistic.Holistic(
            static_image_mode=True,
            model_complexity=2,
            enable_segmentation=True,
            refine_face_landmarks=True,
            min_detection_confidence=0.1,
            min_tracking_confidence=0.1) as holistic:
        for idx, file in enumerate(IMAGE_FILES):
            image = cv2.imread(file)
            image_height, image_width, _ = image.shape
            # Convert the BGR image to RGB before processing.
            results = holistic.process(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))

            if results.pose_landmarks:
                print(
                    f'Nose coordinates: ('
                    f'{results.pose_landmarks.landmark[mp_holistic.PoseLandmark.NOSE].x * image_width}, '
                    f'{results.pose_landmarks.landmark[mp_holistic.PoseLandmark.NOSE].y * image_height})'
                )
            # draw_landmark_annotation_on_the_image_from_static_image(image, results, idx)  # NOTE: for debug now
            landmarks_pose, landmarks_pose_and_hand = extract_data_at_every_frame(image, results, use_hand=USE_HAND,
                                                                                  use_world=False)
            # create_target_bvh(landmarks_pose_and_hand, out_path=f'E:/save/retarget/target{idx}.bvh')  # may change this one
            create_target_bvh(landmarks_pose_and_hand, out_path, use_cuda=use_cuda)


if __name__ == '__main__':
    # For static images:
    images_files = ['assets/c.jpg']  # 暂时用列表存储，因为原始样本是这样，后面根据需求再作变动
    out_path = 'E:/save/retarget/target.bvh'
    get_target_bvh_from_static_image(images_files, out_path, use_cuda=False)
