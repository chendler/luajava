assertThrows('bad argument #1 to \'java.proxy\'',
             java.proxy, 'java.lang.AbsolutelyNotAClass', {})
assertThrows('bad argument #1 to \'java.proxy\'',
             java.proxy, 'java.lang.String', {})
t = {
    value = 1,
    run = function(this)
        this.value = 2
    end
}
runnable = java.proxy('java.lang.Runnable', t)
assert(t.value == 1)
runnable:run()
assert(t.value == 2)

iterImpl = {
  next = function()
    i = i - 1
    return i
  end,
  hasNext = function()
    return i > 0
  end
}

iter = java.proxy('java.util.Iterator', iterImpl)
assertThrows('java.lang.UnsupportedOperationException', iter.remove, iter)
