#pragma once

template <typename T> class Queue 
{
public:

	Queue() {
		front = rear = nullptr;
	}

	~Queue() {
		T temp;
		while (!isEmpty())
			dequeue(temp);
	}

	bool isEmpty() const {
		return front == nullptr && rear == nullptr;
	}

	void enqueue(const T& item) {
		Node* newNode = new Node;
		newNode->data = item;
		newNode->next = nullptr;

		if (isEmpty()) {
			front = rear = newNode;
		}
		else {
			rear->next = newNode;
			rear = newNode;
		}
	}

	bool dequeue(T& dst) {
		if (isEmpty())
			return false;

		Node* temp = front;
		front = front->next;

		if (front == nullptr)
			rear = nullptr;

		dst = temp->data;
		delete temp;

		return true;
	}

private:

	struct Node {
		T data;
		Node* next;
	};

	Node* front;

	Node* rear;

};
