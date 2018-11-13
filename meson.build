project ('pivox', 'c')

gtk_dep = dependency ('gtk+-3.0')

executable ('pivox',
            [
              'pv-application.c',
              'pv-window.c',              
              'main.c',
            ],
            dependencies: [ gtk_dep ],
            install: true)
