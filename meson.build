project('Project Delta', ['cpp', 'c'],
	default_options: [
		'cpp_std=c++17',
		'c_std=c99',
  ])

# Disable warnings as errors
add_project_arguments('-Dwerror=false', language: 'cpp')

dep_thread = dependency('threads')
dep_vulkan = dependency('vulkan')

dep_glfw = dependency('glfw3')
dep_glslang = [dependency('glslang'), dependency('spirv')]
dep_glm = dependency('glm')

# RVG (Statically linked)
dep_dlg = dependency('dlg', fallback: ['dlg', 'dlg_dep'], default_options: ['default_library=static'])
dep_nytl = dependency('nytl', fallback: ['nytl', 'nytl_dep'])
dep_vkpp = dependency('vkpp', fallback: ['vkpp', 'vkpp_dep'], default_options: ['default_library=static'])
dep_katachi = dependency('katachi', fallback: ['katachi', 'katachi_dep'], default_options: ['default_library=static'])
dep_vpp = dependency('vpp', fallback: ['vpp', 'vpp_dep'], default_options: ['default_library=static'])
dep_rvg = dependency('rvg', fallback: ['rvg', 'rvg_dep'], default_options: ['default_library=static'])
deps_rvg = [dep_dlg, dep_nytl, dep_vkpp, dep_katachi, dep_vpp, dep_rvg]

engine_inc = include_directories('.')
engine_dependancies = [dep_vulkan, dep_glfw, dep_glslang, dep_glm, deps_rvg]

subdir('engine')
engine_dep = declare_dependency(
	link_with: engine,
	dependencies: engine_dependancies,
	include_directories: engine_inc)

src = ['main.cpp']
exe_main = executable('main', src,
	dependencies: [engine_dependancies, engine_dep]
)

test('simple test', exe_main)
