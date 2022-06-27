# SPDX-License-Identifier: Apache-2.0

import os
import enum
import time


class RecordStage(enum.Enum):
    INIT = 0,
    WAIT_SHADERS = 1,
    WARMUP = 2,
    RECORD = 3,
    FINISHED = 4


WARMUP_SECONDS = 3
WARMUP_FRAMES = 10
SHADER_FALLBACK_SECONDS = 60
RECORD_SECONDS = 10
LOG_KEY = "ANIMATION_PERFORMANCE: "


def _run(args):
    import bpy

    global record_stage
    record_stage = RecordStage.INIT

    bpy.app.handlers.frame_change_post.append(frame_change_handler)
    bpy.ops.screen.animation_play()

    
def frame_change_handler(scene):
    import bpy
    
    global record_stage
    global start_time
    global start_record_time
    global start_warmup_time
    global num_frames
    global stop_record_time
    
    if record_stage == RecordStage.INIT:
        screen = bpy.context.window_manager.windows[0].screen
        for area in screen.areas:
            if area.type == 'VIEW_3D':
                space = area.spaces[0]
                space.shading.type = 'RENDERED'
                space.overlay.show_overlays = False
        
        start_time = time.perf_counter()
        record_stage = RecordStage.WAIT_SHADERS
    
    elif record_stage == RecordStage.WAIT_SHADERS:
        shaders_compiled = False
        if hasattr(bpy.app, 'is_job_running'):
            shaders_compiled = not bpy.app.is_job_running("SHADER_COMPILATION")
        else:
            # Fallback when is_job_running doesn't exists by waiting for a time.
            shaders_compiled = time.perf_counter() - start_time > SHADER_FALLBACK_SECONDS
        
        if shaders_compiled:
            start_warmup_time = time.perf_counter()
            num_frames = 0
            record_stage = RecordStage.WARMUP
    
    elif record_stage == RecordStage.WARMUP:
        num_frames += 1
        if time.perf_counter() - start_warmup_time > WARMUP_SECONDS and num_frames > WARMUP_FRAMES:
            start_record_time = time.perf_counter()
            num_frames = 0
            record_stage = RecordStage.RECORD
    
    elif record_stage == RecordStage.RECORD:
        current_time = time.perf_counter()
        num_frames += 1
        if current_time - start_record_time > RECORD_SECONDS:
            stop_record_time = current_time
            record_stage = RecordStage.FINISHED

    elif record_stage == RecordStage.FINISHED:
        bpy.ops.screen.animation_cancel()
        elapse_seconds = stop_record_time - start_record_time
        avg_frame_time = elapse_seconds / num_frames
        fps = 1.0 / avg_frame_time
        print(f"{LOG_KEY}{{'time': {avg_frame_time}, 'fps': {fps} }}")
        bpy.app.handlers.frame_change_post.remove(frame_change_handler)
        bpy.ops.wm.quit_blender()
    

if __name__ == '__main__':
    _run(None)
    
else:
    import api
    class EeveeTest(api.Test):
        def __init__(self, filepath):
            self.filepath = filepath

        def name(self):
            return self.filepath.stem

        def category(self):
            return "eevee"

        def run(self, env, device_id):
            args = {}
            _, log = env.run_in_blender(_run, args, [self.filepath], foreground=True)
            for line in log:
                if line.startswith(LOG_KEY):
                    result_str = line[len(LOG_KEY):]
                    result = eval(result_str)
                    return result
            return {'time': 'UNKNOWN'}
        
    def generate(env):
        filepaths = env.find_blend_files('eevee/*')
        return [EeveeTest(filepath) for filepath in filepaths]