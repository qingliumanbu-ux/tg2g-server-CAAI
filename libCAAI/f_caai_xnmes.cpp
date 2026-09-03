/*=========================================================================
//程序名称:     f_caai_xnmes
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：收集生产抛过来的生产数据信息。其中VFREE1（炉号/计划号），Vfree2（R返修即隐形成本）
接口表中入口材料是返修标记，产出用材料来源细分为R
精整：tmmbw28表，包括委外，工序为S1，小工序
炉次确定时间取炼钢计划时间：tpssm41,tpssm42
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

BM2_FUNCTION_EXPORT
int f_caai_xnmes(CString stats_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString sqlstr1 = "";
	CString begin_time = "";
	CString end_time = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	CModel tcaai08("TCAAI08");

	try
	{
		Log::Trace("", "", "dept_code = [{0}],stats_period = [{1}]", dept_code, stats_period);

		sqlstr = "delete from tcaaia1"
			" where 1=1 "
			" and dept_code=@dept_code "
			" and stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		//轧钢大棒,锻造投入实绩和开坯产出实绩，小棒投入实绩
		sqlstr = "insert into tcaaia1(prod_time,  prod_shift_group,prod_shift_no, product_flag, dept_code, equ_no,sub_backlog_code"
			", mat_thick, mat_width, mat_len, remark_specs"
			", sg_sign,SG_STD,  heat_no, st_no, order_no, plan_no,vfree1,vfree2"
			", pro_flag, relation_no, mat_code, qty, wt, key_seq,stats_period,rec_create_time,rec_creator)"
			" select @stats_period||'000000', prod_shift_group, prod_shift_no, product_flag, factory_div,equ_no,case when factory_div='S' then substr(unit_code,1,1)  else whole_backlog_code end"
			",mat_act_thick, mat_act_width, mat_act_len, ingot_code"
			", sg_sign,SG_STD,  heat_no, st_no, order_no, plan_no,relation_no,REPAIR_FLAG "
			", transaction_code, relation_no, product_code"
			", mat_num, wt, 'MM' ||@stats_period||trim(to_char(rownum, '0000000')),@stats_period,@rec_create_time,@rec_creator"
			" from ("
			"  select  acjc_relation_id as relation_no, transaction_code,product_code,DECODE(factory_div,'X',whole_backlog_code||'01',equ_no) EQU_NO,whole_backlog_code,mat_line_type,unit_code"
			", prod_shift_group, prod_shift_no, product_flag, factory_div"			
			" , mat_act_thick, mat_act_width, mat_act_len, ingot_code, mat_shape_flag"
			" , sg_sign,SG_STD, heat_no, st_no, order_no, plan_no,case when MAT_ORIGIN_DETAIL in ('R','WR','W') THEN MAT_ORIGIN_DETAIL ELSE ' ' END AS REPAIR_FLAG"	
			" , sum(mat_num) mat_num, sum(mat_wt) wt"
			" from tmm00ac"
			" where 1=1"
			" and equ_no not like 'W%'"
			//" and  MAT_ORIGIN_DETAIL not in ('WR','W')"
			" and transaction_code in ('I','O')"
			" and balance_date=@stats_period"
			" and factory_div =@factory_div "
			" group by acjc_relation_id, transaction_code,product_code,DECODE(factory_div,'X',whole_backlog_code||'01',equ_no)"
			", prod_shift_group, prod_shift_no, product_flag, factory_div"			
			" , mat_act_thick, mat_act_width, mat_act_len, ingot_code, mat_shape_flag,whole_backlog_code,mat_line_type,unit_code"
			" , sg_sign,SG_STD, heat_no, st_no, order_no, plan_no,case when MAT_ORIGIN_DETAIL in ('R','WR','W') THEN MAT_ORIGIN_DETAIL ELSE ' ' END"
			"  having sum(mat_wt) != 0"
			" )"
			;		
		Log::Trace("", "", "sqlstr新增 = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("rec_create_time", datenow);
		cmd_inq.Parameters.Set("rec_creator", s.userid);
		cmd_inq.Parameters.Set("factory_div", dept_code);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		if (dept_code == "S") //如果是炼钢产线，则因为特殊原因，电炉、LF精炼、RH精炼,使用的产量为连铸产出，则抛帐自动抛入
		{
			sqlstr = "insert into tcaaia1(prod_time,  prod_shift_group,prod_shift_no, product_flag, dept_code, equ_no,sub_backlog_code"
				", mat_thick, mat_width, mat_len, remark_specs"
				", sg_sign,SG_STD,  heat_no, st_no, order_no, plan_no,vfree1"
				", pro_flag, relation_no, mat_code, qty, wt, key_seq,stats_period,rec_create_time,rec_creator)"
				" select prod_time,  prod_shift_group,prod_shift_no, product_flag, dept_code, t2.dev_code,substr(t2.dev_code,1,1)"
				", mat_thick, mat_width, mat_len, remark_specs, sg_sign,SG_STD,  t1.heat_no, st_no, order_no, plan_no,vfree1"
				", pro_flag, relation_no, mat_code, qty, wt, t2.dev_code||key_seq,stats_period,rec_create_time,rec_creator"
				" FROM TCAAIA1 t1"
				" left join (select distinct heat_no,dev_code from tpssm42 where area_id in ('3','4')) t2 on t1.heat_no = t2.heat_no"				
				" where 1=1"
				" and nvl(t2.dev_code,' ')!=' '"
				" and pro_flag = 'O'"				
				" and dept_code = @dept_code"
				" and stats_period = @stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}


		//更新物料名称
		sqlstr = " update tcaaia1 t1 set mat_name=(select mat_name from tcaac11 t2 where t1.mat_code=t2.mat_code) "
			" where 1=1"
			" and exists (select 1 from tcaac11 t2 where t1.mat_code=t2.mat_code)"
			" and  dept_code='" + dept_code + "' and  stats_period='" + stats_period + "' "
			;
		Log::Trace("", "", "sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


		sqlstr = "delete from tcaai08"
			" where 1=1 "
			" and dept_code=@dept_code "
			" and stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();		

		/*sqlstr = " select whole_backlog_code,acjc_relation_id,sg_sign,sum(mat_Wt) mat_wt,PRODUCT_CODE"
			" from tmm00ac"
			" where 1=1"
			" AND whole_backlog_code IN (SELECT SUB_BACKLOG_CODE FROM TCAAC14 WHERE CODE_LINE=@dept_code)"
			" AND transaction_code = 'Q'"
			" and balance_date=@stats_period"			
			" group by balance_date, whole_backlog_code, acjc_relation_id, sg_sign,PRODUCT_CODE"
			" having sum(mat_wt) < 0 "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{			
			tcaai08["TYPE_CODE"] = "Q";
			tcaai08["DEPT_CODE"] = dept_code;
			tcaai08["STATS_PERIOD"] = stats_period;
			tcaai08["PROD_DATE"] = stats_period;
			tcaai08["SUB_BACKLOG_CODE"] = cmd_inq.GetString(1);
			tcaai08["MAT_NO"] = cmd_inq.GetString(2);
			tcaai08["SG_SIGN_OLD"] = cmd_inq.GetString(3);
			tcaai08["PRE_WT"] = 0-cmd_inq.GetDecimal(4);
			tcaai08["PRODUCT_CODE_1"] = cmd_inq.GetString(5);
			

			sqlstr = " select sg_sign,sum(mat_Wt) mat_wt,PRODUCT_CODE"
				" from tmm00ac"
				" where transaction_code = 'Q'"
				" and whole_backlog_code =@whole_backlog_code"
				" and acjc_relation_id=@acjc_relation_id"				
				" and factory_div =@dept_code "
				" and balance_date=@stats_period"
				" group by balance_date, whole_backlog_code, acjc_relation_id, sg_sign,PRODUCT_CODE"
				" having sum(mat_wt) > 0 "
				;
			cmd_inq_sub.SetCommandText(sqlstr);
			cmd_inq_sub.Parameters.Set("dept_code", dept_code);
			cmd_inq_sub.Parameters.Set("stats_period", stats_period);
			cmd_inq_sub.Parameters.Set("whole_backlog_code", cmd_inq.GetString(1));
			cmd_inq_sub.Parameters.Set("acjc_relation_id", cmd_inq.GetString(2));
			cmd_inq_sub.ExecuteReader();
			while (cmd_inq_sub.Read())
			{
				tcaai08["SG_SIGN"] = cmd_inq_sub.GetString(1);
				tcaai08["WT"] = cmd_inq_sub.GetDecimal(2);
				tcaai08["PRODUCT_CODE"] = cmd_inq_sub.GetString(3);
			}
			cmd_inq_sub.Close();

			tcaai08.Insert();
		}
		cmd_inq.Close();*/

		//数据修正
		sqlstr = " insert into tcaai08(TYPE_CODE,DEPT_CODE,STATS_PERIOD"
			" ,PLAN_NO,SUB_BACKLOG_CODE,MAT_THICK,SG_SIGN,WT,PRODUCT_CODE,PROD_SHIFT_NO,PROD_SHIFT_GROUP)"
			" select transaction_code,@dept_code,@stats_period"
			",decode(factory_div,'S',HEAT_NO,SUBSTR(MAT_NO,1,10)),substr(WHOLE_BACKLOG_ACT,length(WHOLE_BACKLOG_ACT)-1,2),MAT_THICK,sg_sign,sum(mat_Wt) mat_wt,PRODUCT_CODE,PROD_SHIFT_NO,PROD_SHIFT_GROUP"
			" from tmm00ac"
			" where 1=1"
			" AND MAT_NO NOT LIKE 'W%'"
			" AND substr(WHOLE_BACKLOG_ACT,length(WHOLE_BACKLOG_ACT)-1,2) IN (SELECT SUB_BACKLOG_CODE FROM TCAAC14 WHERE CODE_LINE=@dept_code)"
			" AND transaction_code in ( 'U','Q','F')"
			" and balance_date=@stats_period"
			" group by transaction_code,balance_date, substr(WHOLE_BACKLOG_ACT,length(WHOLE_BACKLOG_ACT)-1,2), MAT_THICK, sg_sign,PRODUCT_CODE,decode(factory_div,'S',HEAT_NO,SUBSTR(MAT_NO,1,10)),PROD_SHIFT_NO,PROD_SHIFT_GROUP"
			" having sum(mat_wt) != 0 "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
		

		//工时
		sqlstr = " delete from tcaaia14 "
		" where dept_code =@dept_code"
		" and stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		if (dept_code == "S")
		{
			sqlstr = " insert into tcaaia14(REC_CREATE_TIME,pono,RELATION_NO,heat_no,plan_no,dept_code,stats_period,equ_no,SUB_BACKLOG_CODE,ONOFF_TIME,start_time,end_time)"
				" select  @datenow,t2.pono,t1.heat_no,t1.heat_no,t1.sm_plan_no,@dept_code,@stats_period,t1.dev_code,SUBSTR(t1.dev_code,1,1),round(sum(TO_NUMBER((to_date(decode(t1.end_time_real,' ',t1.end_time,t1.end_time_real),'yyyymmddhh24miss')-to_date(decode(t1.start_time_real,' ',t1.start_time,t1.start_time_real),'yyyymmddhh24miss'))* 24 * 60)),0),min(decode(t1.start_time_real,' ',t1.start_time,t1.start_time_real)),max(decode(t1.end_time_real,' ',t1.end_time,t1.end_time_real)) "
				" from tpssm42 t1"
				" left join tpssm41 t2 on t1.heat_no=t2.heat_no"
				" where t1.heat_no in (select heat_no from tpssm42 where start_time_real<=@stats_period||'235959' and start_time_real>=@stats_period||'000000' and  area_id = '5' )"
				" group by t1.heat_no,t1.sm_plan_no,t1.dev_code,t2.pono"
				;
			Log::Trace("", "", "sqlstr={0}", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}
		if (dept_code == "X")
		{
			sqlstr = " insert into tcaaia14(REC_CREATE_TIME,RELATION_NO,heat_no,plan_no,dept_code,stats_period,equ_no,SUB_BACKLOG_CODE,ONOFF_TIME,start_time,end_time,order_no)"
				" select  @datenow,roll_plan_no,heat_no,roll_plan_no,@dept_code,@stats_period,WHOLE_bACKLOG_CODE||'01',WHOLE_bACKLOG_CODE,round(sum((TO_NUMBER(to_date(plan_end_time,'yyyymmddhh24miss')-to_date(plan_start_time,'yyyymmddhh24miss'))* 24 * 60)),0),min(plan_start_time),max(plan_end_time),order_no "
				" from HPSBWA1"
				" where 1=1"
				" and plan_class <> 'B1'"
				" AND  plan_end_time<=@stats_period||'235959' and plan_end_time>=@stats_period||'000000'"
				" group by roll_plan_no,heat_no,equ_no,WHOLE_bACKLOG_CODE,order_no"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			//委外轧制的
			sqlstr = " insert into tcaaia14(REC_CREATE_TIME,RELATION_NO,heat_no,plan_no,dept_code,stats_period,equ_no,SUB_bACKLOG_CODE,ONOFF_TIME,start_time,end_time,ENTRUST_FLAG,order_no)"
				" select  @datenow,plan_no,heat_no,plan_no,@dept_code,@stats_period,equ_no,PLAN_BACKLOG_CODE,round(sum((TO_NUMBER(to_date(plan_end_time,'yyyymmddhh24miss')-to_date(plan_start_time,'yyyymmddhh24miss'))* 24 * 60)),0),min(plan_start_time),max(plan_end_time),ENTRUST_FLAG,order_no "
				" from HPSBWB1"
				" where 1=1"
				" AND PLAN_BACKLOG_CODE IN ('B1','B2','B3','D1')" //轧制的
				" AND  plan_end_time<=@stats_period||'235959' and plan_end_time>=@stats_period||'000000'"
				" group by plan_no,heat_no,equ_no,PLAN_BACKLOG_CODE,ENTRUST_FLAG,order_no"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			//工时更新,最小的轧制时间+最后的轧制时间
			sqlstr = " update tcaaia14 t1 set ONOFF_TIME = (select round((TO_NUMBER(to_date(max(roll_end_time),'yyyymmddhh24miss')-to_date(min(roll_start_time),'yyyymmddhh24miss'))* 24 * 60),0) from tmmbw21b1 t2 where t1.RELATION_NO= t2.roll_plan_no) "
				" where 1=1"
				" and exists (select 1 from tmmbw21b1 t2 where  t1.RELATION_NO= t2.roll_plan_no)"
				" and dept_code = @dept_code"
				" and stats_period =@stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " update tcaaia14 t1 set ONOFF_TIME = (select round((TO_NUMBER(to_date(max(roll_end_time),'yyyymmddhh24miss')-to_date(min(roll_start_time),'yyyymmddhh24miss'))* 24 * 60),0) from tmmbw21b2 t2 where t1.RELATION_NO= t2.roll_plan_no) "
				" where 1=1"
				" and exists (select 1 from tmmbw21b2 t2 where  t1.RELATION_NO= t2.roll_plan_no)"
				" and dept_code = @dept_code"
				" and stats_period =@stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " update tcaaia14 t1 set ONOFF_TIME = (select round((TO_NUMBER(to_date(max(roll_end_time),'yyyymmddhh24miss')-to_date(min(roll_start_time),'yyyymmddhh24miss'))* 24 * 60),0) from tmmbw21b3 t2 where t1.RELATION_NO= t2.roll_plan_no) "
				" where 1=1"
				" and exists (select 1 from tmmbw21b3 t2 where  t1.RELATION_NO= t2.roll_plan_no)"
				" and dept_code = @dept_code"
				" and stats_period =@stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();

			sqlstr = " update tcaaia14 t1 set ONOFF_TIME = (select round((TO_NUMBER(to_date(max(roll_end_time),'yyyymmddhh24miss')-to_date(min(roll_start_time),'yyyymmddhh24miss'))* 24 * 60),0) from tmmbw21d1 t2 where t1.RELATION_NO= t2.roll_plan_no) "
				" where 1=1"
				" and exists (select 1 from tmmbw21d1 t2 where  t1.RELATION_NO= t2.roll_plan_no)"
				" and dept_code = @dept_code"
				" and stats_period =@stats_period"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}
		if (dept_code == "R")
		{
			sqlstr = " insert into tcaaia14(REC_CREATE_TIME,RELATION_NO,heat_no,plan_no,dept_code,stats_period,equ_no,SUB_bACKLOG_CODE,ONOFF_TIME,start_time,end_time,ENTRUST_FLAG,order_no)"
				" select  @datenow,plan_no,heat_no,plan_no,@dept_code,@stats_period,equ_no,PLAN_BACKLOG_CODE,round(sum((TO_NUMBER(to_date(plan_end_time,'yyyymmddhh24miss')-to_date(plan_start_time,'yyyymmddhh24miss'))* 24 * 60)),0),min(plan_start_time),max(plan_end_time),ENTRUST_FLAG,order_no "
				" from HPSBWB1"
				" where 1=1"
				" AND PLAN_BACKLOG_CODE NOT IN ('B1','B2','B3','D1')" //轧制的
				" AND  plan_end_time<=@stats_period||'235959' and plan_end_time>=@stats_period||'000000'"
				" group by plan_no,heat_no,equ_no,PLAN_BACKLOG_CODE,ENTRUST_FLAG,order_no"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("datenow", datenow);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}

		//更新计划号、设备号、计产日期
		sqlstr = " update tcaai08 t1 set (equ_no,prod_date)= (select equ_no,stats_period from tcaaia14 t2 where t1.plan_no=t2.plan_no and t2.sub_Backlog_code=decode(@dept_code,'S','C',t2.sub_Backlog_code))"
			" where exists(select 1 from tcaaia14 t2 where t1.plan_no=t2.plan_no and t2.sub_Backlog_code=decode(@dept_code,'S','C',t2.sub_Backlog_code))"
			" and dept_code = @dept_code"
			" and stats_period = @stats_period"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		
		
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。" /* _RES("GCRSS0000006")*//*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		EDLog(1, 1, "[%s]", s.sysmsg);
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}
