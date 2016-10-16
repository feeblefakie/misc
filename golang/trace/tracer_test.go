package trace

import (
	"bytes"
	"testing"
)

func TestNew(t *testing.T) {
	var buf bytes.Buffer
	tracer := New(&buf)
	if tracer == nil {
		t.Error(("New returns nil"))
	} else {
		//tracer.Trace("hello trace package")
		tracer.Trace("hello trace package")
		if buf.String() != "hello trace package\n" {
			t.Errorf("'%s' is output", buf.String())
		}
	}
}

func TestOff(t *testing.T) {
	var silentTracer Tracer = Off()
	silentTracer.Trace("data")
}
