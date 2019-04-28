import cProfile, pstats, io

pr = None

class TimeTester(object):
    def __init__(self):
        global pr
        pr = cProfile.Profile()
        pr.enable()

    def report(self):
        pr.disable()
        s = io.StringIO()
        ps = pstats.Stats(pr, stream=s).sort_stats('cumulative')
        ps.print_stats()
        print(s.getvalue())