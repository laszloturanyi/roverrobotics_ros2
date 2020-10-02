from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import ThisLaunchFileDir
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node


def generate_launch_description():
    rplidar_node = Node(
        node_name='rplidarNode',
        package='rplidar_ros',
        node_executable='rplidarNode',
        output='screen',
        parameters=[{
                'serial_port': '/dev/rplidar',
                'serial_baudrate': 256000,
                'frame_id': 'laser',
                'inverted': False,
                'angle_compensate': True,
        }],
    )
    return LaunchDescription([
        Node(
            node_name='rplidarNode',
            package='rplidar_ros',
            node_executable='rplidarNode',
            output='screen',
            parameters=[{
                'serial_port': '/dev/rplidar',
                'serial_baudrate': 256000,
                'frame_id': 'laser',
                'inverted': False,
                'angle_compensate': True,
            }],
        ),
    ])
