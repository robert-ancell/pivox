project ('pivox', 'c')

cc = meson.get_compiler ('c')
gnome = import ('gnome')

epoxy_dep = dependency ('epoxy')
gtk_dep = dependency ('gtk+-3.0')
json_glib_dep = dependency ('json-glib-1.0')
m_dep = cc.find_library ('m', required: false)

resources = gnome.compile_resources ('pv-resources',
                                     'pv-resources.gresource.xml',
                                     c_name: 'pv_resources')

executable ('pivox',
            [
              'pv-application.c',
              'pv-camera.c',
              'pv-map.c',
              'pv-map-generator.c',
              'pv-map-generator-default.c',
              'pv-renderer.c',
              'pv-vox-file.c',
              'pv-window.c',
              'main.c',
            ] + resources,
            dependencies: [ epoxy_dep, gtk_dep, json_glib_dep, m_dep ],
            install: true)
