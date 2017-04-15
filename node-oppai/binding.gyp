{
    'targets': [
        {
            'target_name': 'oppai',
            'sources': [
                'node-oppai.cc'
            ],
            #'defines': [ 'OPPAI_MODULE_DEBUG=1' ]

            'conditions': [
                [
                    'OS=="win"',
                    {
                        'defines': ['NOMINMAX=1', '_CRT_SECURE_NO_WARNINGS=1'],
                        'cflags_cc+': ['-F8000000', '-wd4201', '-wd4100'],
                    }
                ]
            ]
        }
    ]
}
