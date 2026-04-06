has_exited = False
def stop_handler (event):
  global has_exited
  if isinstance(event, gdb.SignalEvent):
    print("exit code: %s" % (event.stop_signal))
    has_exited = True
gdb.events.stop.connect (stop_handler)
gdb.execute('continue')
while not has_exited:
  r = gdb.execute('when', to_string=True)
  m = re.match("[^0-9]*([0-9]+)", r)
  if m:
    event = int(m.group(1));
    gdb.execute('start ' + str(event + 1));
    gdb.execute('continue')
gdb.events.stop.disconnect (stop_handler)
gdb.execute('file ' + str(gdb.parse_and_eval("$etp_beam_executable")))
gdb.execute('break main')
gdb.execute('reverse-continue')