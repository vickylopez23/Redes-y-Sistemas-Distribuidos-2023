simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        volatile int packetByteSize; // Tamaño del paquete
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input in;
        output out;
}

simple Sink
{
    parameters:
        @display("i=block/sink");
    gates:
        input in;
}

module NodeTx
{
    parameters:
        @display("i=block/routing");
    gates:
        output out;
    submodules:
        gen: Generator{
            parameters:
                @display("p=75,50");
        }
        queue: Queue{
            parameters:
                @display("p=75,190");
        }
    connections:
        gen.out --> queue.in;
        queue.out --> out;
}

module NodeRx
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
    submodules:
        sink: Sink{
            parameters:
                @display("p=75,50");
        }
        queue: Queue{
            parameters:
                @display("p=75,190");
        }
    connections:
        in --> queue.in;
        // caso de estudio 1
        queue.out --> {datarate = 0.5Mbps;} --> sink.in;
        // caso de estudio 2
//        queue.out --> {datarate = 1Mbps;} --> sink.in;
}

network Network
{
    @display("bgl=2");
    submodules:
        nodetx: NodeTx {
            @display("p=30,30");
        }
        queue: Queue {
            @display("p=130,30");
        }
        noderx: NodeRx {
            @display("p=230,30");
        }
    connections:
        nodetx.out --> {datarate = 1Mbps; delay = 100us;} --> queue.in;
        // caso de estudio 1
        queue.out --> {datarate = 1Mbps; delay = 100us;} --> noderx.in;
        // caso de estudio 2
//        queue.out --> {datarate = 0.5Mbps; delay = 100us;} --> noderx.in;
}
