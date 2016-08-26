/* main test simulator */
#include <iostream>
#include "CraigUtils.h"
#include "Window.h"
#include "tokens.h"
#include "ObjInfo.h"
#include "QuadTree.h" 
#include "Params.h"
#include "LifeForm.h"
#include "Event.h"
#include "Random.h"

using namespace std;

template <typename T>
void bound(T& x, const T& min, const T& max) {
	assert(min < max);
	if (x > max) { x = max; }
	if (x < min) { x = min; }
}



ObjInfo LifeForm::info_about_them(SmartPointer<LifeForm> neighbor) {
	ObjInfo info;

	info.species = neighbor->species_name();
	info.health = neighbor->health();
	info.distance = pos.distance(neighbor->position());
	info.bearing = pos.bearing(neighbor->position());
	info.their_speed = neighbor->speed;
	info.their_course = neighbor->course;
	return info;
}

void LifeForm::update_position(void) {
	if (!is_alive)return;
	double time = Event::now() - update_time;
	if (time <= 0.001) return;
	/*if (space.is_out_of_bounds(this->pos)) {
		this->die();
		return;
	}*/
	double distance = speed*time;
	Point oldpos;
	oldpos.xpos = pos.xpos;
	oldpos.ypos = pos.ypos;
	Point newpos;
	newpos.xpos = pos.xpos + distance*cos(course);
	newpos.ypos = pos.ypos + distance*sin(course);
	if (space.is_out_of_bounds(newpos)) {
		this->die();
		return;
	}
	update_time = Event::now();
	energy -= movement_cost(speed, time);
	if (energy < min_energy) {
		this->die();
		return;
	}
	pos.xpos = newpos.xpos;
	pos.ypos = newpos.ypos;
	if (space.is_out_of_bounds(this->pos)) {
		this->die();
		return;
	}
	space.update_position(oldpos, newpos);
}

void LifeForm::region_resize(void) {
	if (!is_alive)return;
	this->border_cross_event->cancel();
	this->update_position();
	if(speed>0){
	this->compute_next_move();
	}
}

void LifeForm::compute_next_move(void) {
	if (speed < 0) {
		this->die();
		return;
	}
	if (!is_alive)return;
	this->update_position();
	double distance_to_edge = space.distance_to_edge(pos, course);
	double time = distance_to_edge / speed + Point::tolerance;
	SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
	border_cross_event = new Event(time, [self](void) {self->border_cross(); });
	
}

void LifeForm::border_cross(void) {
	if (speed < 0) {
		this->die();
		return;
	}
	if (!is_alive)return;
	this->update_position();
	this->check_encounter();
	if(speed>0){
	this->compute_next_move();
	}

}

void LifeForm::check_encounter(void) {
	if (!is_alive)return;
	update_position();
	SmartPointer<LifeForm> y = space.closest(pos);//zenme xiede???
	y->update_position();
	double xy_distance = this->pos.distance(y->position());
	if (xy_distance < 1.0) {
		this->resolve_encounter(y);

	}

}
void LifeForm::set_speed(double speed) {
	if (!is_alive)return;
	
	if (speed >= max_speed) {
		border_cross_event->cancel();
		this->update_position();
		this->speed = max_speed;
		border_cross_event->cancel();//jianchabordershibushikong
		if(speed>0){
			this->compute_next_move();
		}
		
	}
	else {
		this->update_position();
		this->speed = speed;
		border_cross_event->cancel();
		if(speed>0){
	
	    this->compute_next_move();
		}
	}
}

void LifeForm::set_course(double course) {
	if (!is_alive)return;
	border_cross_event->cancel();
	this->update_position();
	this->course = course;

	border_cross_event->cancel();
	if(speed>0){
	this->compute_next_move();
	}

}

void LifeForm::gain_energy(double extraenergy) {
	update_position();
	if (!is_alive)return;
	if (energy < min_energy) {
		this->die();
		return;
	}
	this->energy += eat_efficiency*extraenergy;
	if (energy < min_energy) {
		this->die();
		return;
	}

}
void LifeForm::eat(SmartPointer<LifeForm> y) {
	update_position();

	if (!is_alive)return;
	y->die();
	double notUsed0 = 0;
	double notUsed1 = 0;
	this->energy -= eat_cost_function( notUsed0 ,  notUsed1);
	if (energy < min_energy) {
		this->die();
		return;
	}
	SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
	new Event(digestion_time, [=](void) {self->gain_energy(y->energy); });
}
void LifeForm::resolve_encounter(SmartPointer<LifeForm> y) {
	update_position();

	if (!is_alive)return;

	if (!(y->is_alive))return;
	
	energy -= encounter_penalty;
	y->energy -= encounter_penalty;
    if (energy < min_energy) {
		this->die();
		return;
	}
	if (y->energy < min_energy) {
		y->die();
		return;
	}
    Action act1 = this->encounter(this->info_about_them(y));
	Action act2 = y->encounter(y->info_about_them(this));
	if ((act1 == LIFEFORM_IGNORE) && (act2 == LIFEFORM_IGNORE)) {
		return;
	}
	else if ((act1 == LIFEFORM_EAT) && (act2 == LIFEFORM_IGNORE)) {
		double n1 = epl::drand48();
		double n2 = eat_success_chance(this->energy, y->energy);
		if (n1 < n2) {
			this->eat(y);
		}
	}
	else if ((act1 == LIFEFORM_IGNORE) && (act2 == LIFEFORM_EAT)) {
		double n1 = epl::drand48();
		double n2 = eat_success_chance(y->energy, this->energy);
		if (n1 < n2) {
			y->eat(this);
		}
	}
	else if ((act1 == LIFEFORM_EAT) && (act2 == LIFEFORM_EAT)) {
		double n1 = epl::drand48();
		double n2 = eat_success_chance(this->energy, y->energy);
		bool xeaty = (n1 < n2);
		double n3 = epl::drand48();
		double n4 = eat_success_chance(y->energy, this->energy);
		bool yeatx = (n3 < n4);
		if ((xeaty == 1) && (yeatx == 1)) {
			switch (encounter_strategy)
			{
			case BIG_GUY_WINS: {
				if (this->energy > y->energy) { this->eat(y); }
				else { y->eat(this); }
				break;
			}
			case UNDERDOG_IS_HERE: {
				if (this->energy < y->energy) { this->eat(y); }
				else { y->eat(this); }
				break;
			}
			case FASTER_GUY_WINS: {
				if (this->speed > y->speed) { this->eat(y); }
				else { y->eat(this); }
				break;
			}
			case SLOWER_GUY_WINS: {
				if (this->speed < y->speed) { this->eat(y); }
				else { y->eat(this); }
				break;
			}
			case EVEN_MONEY: {
				double n = epl::drand48();
				if (n > 0.5) { this->eat(y); }
				else { y->eat(this); }
			}
			}//how to implement break tie strategy
		}
		else if ((xeaty == 1) && (yeatx == 0)) {
			this->eat(y);
		}
		else if ((xeaty == 0) && (yeatx == 1)) {
			y->eat(this);
		}
	}
	
}

void LifeForm::age(void) {

	update_position();
	if (!is_alive)return;
	this->energy -= age_penalty;
	if (energy < min_energy) {
		this->die();
		return;
	}
	SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
	new Event(age_frequency, [self](void) {self->age(); });

}

void LifeForm::reproduce(SmartPointer<LifeForm>child) {
	update_position();
	
	if (!is_alive) {
		child = nullptr;
		return;
	}
	if (energy < min_energy) {
		this->die();
		child = nullptr;
		return;
	}
	double time = Event::now() - reproduce_time;
	if (time <= min_reproduce_time) {
		child = nullptr;
		return;
	}
	
	double dist = reproduce_dist;
	Point childoldpos;
	childoldpos.xpos = child->pos.xpos;
	childoldpos.ypos = child->pos.ypos;
	
	int i = 0;
		while (i < 5) {
			double childdistance = reproduce_dist*epl::drand48();
		    child->course = epl::drand48() * 2 * M_PI;
			child->pos.xpos = this->pos.xpos + childdistance*cos(child->course);
			child->pos.ypos=this->pos.ypos+ childdistance*sin(child->course);
			SmartPointer<LifeForm> y = space.closest(child->pos);
			double childtoother_distance = child->pos.distance(y->position());
			if ((childtoother_distance < 1.0) || (space.is_out_of_bounds(child->pos))||space.is_occupied(child->pos)) {
				i++;
			}
			else break;
		}
		bool c1 = space.is_out_of_bounds(child->pos);
		SmartPointer<LifeForm> y = space.closest(child->pos);
		double childtoother_distance = child->pos.distance(y->position());
		bool occupy = space.is_occupied(child->pos);
		if ((childtoother_distance < 1.0) || (space.is_out_of_bounds(child->pos))||occupy) {
			child = nullptr;
			return;
		}
	
	
		child->is_alive = 1;
		child->energy = this->energy / 2 * (1 - reproduce_cost);
		space.insert(child, child->pos, [child]() { child->region_resize(); });
	
		this->energy = this->energy / 2 * (1 - reproduce_cost);
	    reproduce_time = Event::now();
		if (energy < min_energy) {
		this->die();
		}
		if (child->energy < min_energy) {
			child->die();
		}
}
ObjList LifeForm::perceive(double perceive_range) {
	update_position();

	ObjList zero;
	if (!is_alive)return zero ;
	
	double perceive_range_new;
	if (perceive_range > max_perceive_range) {
		perceive_range_new = max_perceive_range;
	}
	else if (perceive_range < min_perceive_range) {
		perceive_range_new = min_perceive_range;
	}
	else perceive_range_new = perceive_range;
	double perceive_energy_coust = perceive_cost(perceive_range_new);
	this->energy -= perceive_energy_coust;
	if (energy < min_energy) {
		this->die();
		return zero;
	}
	
	vector<SmartPointer<LifeForm>> neighbor = space.nearby(this->pos, perceive_range_new);

	ObjList returnvalue;
	for (uint32_t k = 0; k < neighbor.size(); k += 1) {

		if (neighbor[k]->is_alive) {
			ObjInfo x = info_about_them(neighbor[k]);
			returnvalue.push_back(x);
		}
	}

	update_position();
	return returnvalue;

}





