# Copyright (c) 2021-2022, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.

import launch
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    """Generate launch description for TensorRT ROS2 node."""
    # By default loads and runs mobilenetv2-1.0 included in isaac_ros_dnn_inference/models
    launch_args = [
        DeclareLaunchArgument(
            'model_file_path',
            default_value='',
            description='The absolute file path to the ONNX file'),
        DeclareLaunchArgument(
            'engine_file_path',
            default_value='',
            description='The absolute file path to the TensorRT engine file'),
        DeclareLaunchArgument(
            'input_tensor_names',
            default_value='["input"]',
            description='A list of tensor names to bound to the specified input binding names'),
        DeclareLaunchArgument(
            'input_binding_names',
            default_value='["data"]',
            description='A list of input tensor binding names (specified by model)'),
        DeclareLaunchArgument(
            'output_tensor_names',
            default_value='["output"]',
            description='A list of tensor names to bound to the specified output binding names'),
        DeclareLaunchArgument(
            'output_binding_names',
            default_value='["mobilenetv20_output_flatten0_reshape0"]',
            description='A  list of output tensor binding names (specified by model)'),
        DeclareLaunchArgument(
            'verbose',
            default_value='False',
            description='Whether TensorRT should verbosely log or not'),
        DeclareLaunchArgument(
            'force_engine_update',
            default_value='False',
            description='Whether TensorRT should update the TensorRT engine file or not'),
    ]

    # TensorRT parameters
    model_file_path = LaunchConfiguration('model_file_path')
    engine_file_path = LaunchConfiguration('engine_file_path')
    input_tensor_names = LaunchConfiguration('input_tensor_names')
    input_binding_names = LaunchConfiguration('input_binding_names')
    output_tensor_names = LaunchConfiguration('output_tensor_names')
    output_binding_names = LaunchConfiguration('output_binding_names')
    verbose = LaunchConfiguration('verbose')
    force_engine_update = LaunchConfiguration('force_engine_update')

    tensor_rt_node = ComposableNode(
        name='tensor_rt',
        package='isaac_ros_tensor_rt',
        plugin='nvidia::isaac_ros::dnn_inference::TensorRTNode',
        remappings=[('tensor_pub', 'tensor_pub'),
                    ('tensor_sub', 'tensor_sub')],
        parameters=[{
            'model_file_path': model_file_path,
            'engine_file_path': engine_file_path,
            'output_binding_names': output_binding_names,
            'output_tensor_names': output_tensor_names,
            'input_tensor_names': input_tensor_names,
            'input_binding_names': input_binding_names,
            'verbose': verbose,
            'force_engine_update': force_engine_update
        }]
    )

    tensor_rt_container = ComposableNodeContainer(
        name='tensor_rt_container',
        package='rclcpp_components',
        executable='component_container_mt',
        composable_node_descriptions=[tensor_rt_node],
        namespace='isaac_ros_tensor_rt'
    )

    final_launch_description = launch_args + [tensor_rt_container]
    return launch.LaunchDescription(final_launch_description)
