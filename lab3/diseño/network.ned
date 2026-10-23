simple Generator
{
    parameters:
        volatile double generationInterval; // sec
        volatile int packetByteSize; //tamaño del paquete
        @display("i=block/source");
    gates:
        output out;
}

simple Queue
{
    parameters:
        volatile int bufferSize @display("i=block/queue;q=buffer");
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
        input in;
        output out;
    submodules:
        gen: Generator {
            parameters:
                @display("p=75,50");
        }
        traTx: TransportTx {
            parameters:
                @display("p=75,190");
        }
    connections:
        in --> traTx.toOut$i;
        gen.out --> traTx.toApp;
        traTx.toOut$o --> out;
}

module NodeRx
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
    submodules:
        sink: Sink {
            parameters:
                @display("p=75,50");
        }
        traRx: TransportRx {
            parameters:
                @display("p=75,190");
        }
    connections:

        in --> traRx.toOut$i;
        traRx.toOut$o --> out;

        // caso de estudio 1
//        traRx.toApp --> {  datarate = 0.5Mbps; } --> sink.in;
        // caso de estudio 2
         traRx.toApp --> {datarate = 1Mbps;} --> sink.in;
}

simple TransportTx
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        input toApp;
        inout toOut;
}

simple TransportRx
{
    parameters:
        int bufferSize;
        @display("i=block/queue;q=buffer");
    gates:
        output toApp;
        inout toOut;
}

network Network
{
    @display("bgl=2");
    submodules:
        nodetx: NodeTx {
            @display("p=30,30");
        }
        queue0: Queue {
            @display("p=130,30");
        }
        queue1: Queue {
            @display("p=130,104");
        }
        noderx: NodeRx {
            @display("p=230,30");
        }
    connections:
        nodetx.out --> {  datarate = 1Mbps; delay = 100us; } --> queue0.in;
        // caso de estudio 1
//        queue0.out --> {  datarate = 1Mbps; delay = 100us; } --> noderx.in;
        // caso de estudio 2
        queue0.out --> {datarate = 0.5Mbps; delay = 100us;} --> noderx.in;


        // vuelta
        noderx.out --> {  datarate = 1Mbps; delay = 100us; } --> queue1.in;
        // caso de estudio 1
//        queue1.out --> {  datarate = 1Mbps; delay = 100us; } --> nodetx.in;
        // caso de estudio 2
        queue1.out --> {datarate = 0.5Mbps; delay = 100us;} --> nodetx.in;
}
