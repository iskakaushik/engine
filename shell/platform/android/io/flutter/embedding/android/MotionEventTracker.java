package io.flutter.embedding.android;

import android.view.MotionEvent;
import androidx.annotation.Nullable;
import java.util.concurrent.atomic.AtomicLong;
import java.util.HashMap;
import java.util.Map;

/**
 * Tracks the motion events received by the FlutterView.
 */
public final class MotionEventTracker {

    /**
     * Represents a unique identifier corresponding to a motion event.
     */
    public static class MotionEventId {
        private static final AtomicLong ID_COUNTER = new AtomicLong(0);
        private final long id;

        private MotionEventId(long id) {
            this.id = id;
        }

        public static MotionEventId from(long id) {
            return new MotionEventId(id);
        }

        public static MotionEventId createUnique() {
            return MotionEventId.from(ID_COUNTER.incrementAndGet());
        }

        public boolean equals(Object object) {
            if (this == object) return true;
            if (object == null || getClass() != object.getClass()) return false;
            if (!super.equals(object)) return false;

            MotionEventId that = (MotionEventId) object;

            if (id != that.id) return false;

            return true;
        }

        public int hashCode() {
            int result = super.hashCode();
            result = 31 * result + (int) (id ^ (id >>> 32));
            return result;
        }

        public long getId() {
            return id;
        }
    }

    private final Map<MotionEventId, MotionEvent> eventById;
    private static MotionEventTracker INSTANCE;

    public static MotionEventTracker getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new MotionEventTracker();
        }
        return INSTANCE;
    }

    private MotionEventTracker() {
        eventById = new HashMap<>();
    }

    /**
     * Tracks the event and returns a unique MotionEventId identifying the event.
     */
    public MotionEventId track(MotionEvent event) {
        MotionEventId eventId = MotionEventId.createUnique();
        eventById.put(eventId, event);
        return eventId;
    }

    /**
     * Returns the MotionEvent corresponding to the eventId while discarding all the motion events
     * that occured prior to the event represented by the eventId. Returns null if this event was popped
     * or discarded.
     */
    @Nullable
    public MotionEvent pop(MotionEventId eventId) {
        // TODO(kaushikiska) do the actual timestamp based book-keeping.
        return eventById.remove(eventId);
    }

}