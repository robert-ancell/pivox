project ('pivox', 'c')

gnome = import ('gnome')

epoxy_dep = dependency ('epoxy')
gtk_dep = dependency ('gtk+-3.0')

resources = gnome.compile_resources ('pv-resources',
                                     'pv-resources.gresource.xml',
                                     c_name: 'pv_resources')

executable ('pivox',
            [
              'pv-application.c',
              'pv-block-type.c',
              'pv-map.c',
              'pv-renderer.c',
              'pv-window.c',
              'main.c',
            ] + resources,
            dependencies: [ epoxy_dep, gtk_dep ],
            install: true)
