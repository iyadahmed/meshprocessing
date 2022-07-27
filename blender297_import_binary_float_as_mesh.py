import bpy
import numpy as np

# Change path
arr = np.fromfile("/home/iyad/Desktop/meshprocessing/build/foo.pts", dtype=np.float32)
arr.shape = -1, 3
print(arr)

mesh = bpy.data.meshes.new("")
mesh.from_pydata(arr, [], [])
obj = bpy.data.objects.new("", mesh)
bpy.context.scene.objects.link(obj)
bpy.context.scene.update()